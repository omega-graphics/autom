#include <iostream>
#include <map>
#include "ExecEngine.h"
#include "Toolchains.def"
#include "Diagnostic.h"

namespace autom {
    ExecEngine::ExecEngine(ExecEngineOpts &opts,OutputTargetOpts & outputTargetOpts) :
    lexer(std::make_unique<Lexer>()),
    astFactory(std::make_unique<ASTFactory>(*lexer)),
    exec(std::make_unique<eval::Eval>(opts.gen,this)),
    opts(opts),
    outputTargetOpts(outputTargetOpts){
        
        astFactory->engine = this;
        
        if(!std::filesystem::exists(opts.toolchainFile.data())){
            printError(formatmsg("Toolchain file not found! (@0)",opts.toolchainFile));
            exit(1);
        }
        ToolchainLoader loader{opts.toolchainFile};

        std::string preferedToolchain;

        ToolchainSearchOpts searchOpts {preferedToolchain,true,ToolchainSearchOpts::ccAsmFamily,outputTargetOpts.platform};
            
        toolchain = loader.getToolchain(searchOpts);
        if(!toolchain){
            printError("Failed to find working toolchain");
            exit(1);
        }
        else if(!toolchain->verifyTools()) {
            printError("Failed to find working toolchain");
            exit(1);
        }
        
        
        std::vector<std::pair<std::string,Object *>> autom_obj = {
            
            {"c_flags",new eval::Array()},
            {"cxx_flags",new eval::Array()},
            
            {"toolchain",new eval::String(toolchain->name)},
            {"target_os",new eval::String(TargetOSToString(outputTargetOpts.os))},
            {"target_arch",new eval::String(TargetArchToString(outputTargetOpts.arch))},
            {"target_platform",new eval::String(TargetPlatformToString(outputTargetOpts.platform))}
        };
        
        exec->setGlobalVar("autom",new eval::Namespace(autom_obj));
    };

    bool ExecEngine::parseAndEvaluate(std::istream * in){
        std::vector<Tok> tokenVector;

        lexer->setInputStream(in);
        lexer->tokenize(&tokenVector);   
        lexer->finish();
        
        astFactory->setTokenVector(&tokenVector);   

        ASTNode *node;
        bool success = true;
        while((node = astFactory->nextStmt()) != nullptr){
            if(!exec->evalStmt(node)){
                success = false;
                break;
            };
        };
        
        return success;
        
    };

    bool ExecEngine::checkDependencyTree(){

        if(exec->targets.empty()){
            printError("No targets have been created in this project!");
            return false;
        }
        
        bool success = true;

        std::vector<std::pair<autom::StrRef,Target *>> graph;
        for(auto & t : exec->targets){

            if(t->type & COMPILED_OUTPUT_TARGET){

                auto *compiledTarget = (CompiledTarget *)t;
                /// 1. Put Sources into map
                for(auto s_it =  compiledTarget->srcs->getBeginIterator();s_it != compiledTarget->srcs->getEndIterator();s_it++){
                    /// Resolve Source so that it is relative to output dir!
                    auto fixed_source = std::filesystem::path(eval::castToString(*s_it)->value().data()).lexically_relative(opts.outputDir.data());
                    
                    compiledTarget->source_object_map.insert(std::make_pair(fixed_source.string(),""));
                    
                }
                /// 2.  Check sources count!
                if(compiledTarget->source_object_map.empty()){
                    printError(formatmsg("Target `@0` has no sources",compiledTarget->name->value()));
                    return false;
                }
                
                /// 3.Resolve Object Files
                for(auto & src_obj_map : compiledTarget->source_object_map){
                    auto src_path =  std::filesystem::path(src_obj_map.first);

                    auto src_name = src_path.filename();

                    if(outputTargetOpts.os == TargetOS::Windows)
                        src_name.replace_extension("obj");
                    else
                        src_name.replace_extension("o");

                    src_obj_map.second = std::filesystem::path("obj").append(compiledTarget->name->value().data()).append(src_name.c_str()).string();
                }
            }
            
            auto resolveTargetForKey = [&](const StrRef & key){
                auto graph_it = graph.begin();
                
                for(;graph_it != graph.end();graph_it++){
                    if(graph_it->first == key){
                        break;
                    }
                }
                
                return graph_it;
            };
            
            std::vector<autom::StrRef> unresolvedDepNames;
            
            /// 4. Resolve Dependencies using the Target Graph

            for(auto dep : t->deps->value()){
                auto dep_name = eval::castToString(dep)->value();
                auto dep_resolved_it = resolveTargetForKey(dep_name);
                if(dep_resolved_it == graph.end()){
                    unresolvedDepNames.push_back(dep_name);
                }
                else {
                    t->resolvedDeps.push_back(dep_resolved_it->second);
                }
            }
            
            if(unresolvedDepNames.empty()) {
                /// All Dependencies are resolved therefore this Target can be added to the Target Graph.
                graph.emplace_back(std::make_pair(t->name->value(),t));
            }
            else {
                std::ostringstream names;
                for(auto n = unresolvedDepNames.begin();n != unresolvedDepNames.end();n++){
                    if(n != unresolvedDepNames.begin()){
                        names << " , ";
                    }
                    names << "`" << *n << "`";
                }
                
                /// Logs missing dependencies!  NOTE:
                /// This invocation is inside the target iteration loop therefore all targets with missing dependencies will be reported!
                
                printError(formatmsg("Target `@0` has unresolved dependencies: @1",t->name->value(),names.str()));
                
                success = false;
            }

        }

        return success;
    };

    void ExecEngine::generate(){
        
        auto autom_ns = eval::castToNamespace(exec->referVarWithScope(GLOBAL_SCOPE,"autom"));
        
        ToolchainDefaults defs {
            
            eval::castToArray(autom_ns->get("c_flags")),
            eval::castToArray(autom_ns->get("cxx_flags"))
            
        };
        
        opts.gen.consumeToolchainDefaults(defs);
        
        if(opts.gen.supportsCustomToolchainRules()){
            opts.gen.genToolchainRules(toolchain);
        };
        while(!exec->targets.empty()){
            auto t = exec->targets.front();
            opts.gen.consumeTarget(t);
            exec->targets.pop_front();
            delete t;
        };
        opts.gen.finish();
    };

    ExecEngine::~ExecEngine(){
        lexer->finish();
    }

    unsigned ExecEngine::resetASTFactoryTokenIndex(unsigned new_value) {
        auto old_val = astFactory->privTokIndex;
        astFactory->privTokIndex = new_value;
        return old_val;
    };

    std::vector<Tok> * ExecEngine::resetASTFactoryTokenVector(std::vector<Tok> *new_vec){
        auto old_val = astFactory->tokStream;
        astFactory->setTokenVector(new_vec);
        return old_val;
    }

    void ExecEngine::printError(const std::string& msg) {
        std::cout << "\x1b[31mERROR:\x1b[0m" << msg << std::endl;
    }

    void ExecEngine::report(){
        std::cout << "\x1b[33mSuccess!\x1b[0m " << exec->targetCount;
        
        if(exec->targetCount == 1){
            std::cout << " target has been generated!" << std::endl;
        }
        else {
            std::cout << " targets have been generated!" << std::endl;
        }
    }


};
