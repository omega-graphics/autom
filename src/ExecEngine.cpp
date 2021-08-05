#include <iostream>
#include <map>
#include "ExecEngine.h"
#include "Toolchains.def"
#include "Diagnostic.h"

namespace autom {
    ExecEngine::ExecEngine(ExecEngineOpts &opts,OutputTargetOpts & outputTargetOpts) :
    opts(opts),
    lexer(std::make_unique<Lexer>()),
    astFactory(std::make_unique<ASTFactory>(*lexer)),
    exec(std::make_unique<eval::Eval>(opts.gen,this)),outputTargetOpts(outputTargetOpts),toolchain(std::make_shared<Toolchain>()){
        toolchain->toolchainType = TOOLCHAIN_LLVM;
        toolchain->CC.command = LLVM_CLANGMSVC;
        toolchain->CXX.command = LLVM_CLANGMSVC;
        toolchain->EXE_LD.command = LLVM_LLDLINK;
        toolchain->SO_LD.command = LLVM_LLDLINK;
        toolchain->AR.command = MSVC_LIB;
    };

    void ExecEngine::parseAndEvaluate(std::istream * in){
        std::vector<Tok> tokenVector;

        lexer->setInputStream(in);
        lexer->tokenize(&tokenVector);   
        lexer->finish();
        
        astFactory->setTokenVector(&tokenVector);   

        ASTNode *node;
        while((node = astFactory->nextStmt()) != nullptr){
            if(!exec->evalStmt(node)){
                break;
            };
        };
        
    };

    bool ExecEngine::checkDependencyTree(){

        if(exec->targets.empty()){
            printError("No targets have been created in this project!");
            return false;
        }

        std::vector<std::pair<autom::StrRef,Target *>> graph;
        for(auto & t : exec->targets){

            if(t->type & COMPILED_OUTPUT_TARGET){
             

                auto *compiledTarget = (CompiledTarget *)t;
                /// Put Sources into map
                for(auto s_it =  compiledTarget->srcs->getBeginIterator();s_it != compiledTarget->srcs->getEndIterator();s_it++){
                    compiledTarget->source_object_map.insert(std::make_pair(eval::castToString(*s_it)->value(),""));
                }
                /// 1.  Check sources count!
                if(compiledTarget->source_object_map.empty()){
                    printError(formatmsg("Target `@0` must has no sources!",compiledTarget->name->value()));
                    return false;
                }
                /// 2.Resolve Object Files
                for(auto & src_obj_map : compiledTarget->source_object_map){
                    auto src_name = std::filesystem::path(src_obj_map.first).filename();
                    if(outputTargetOpts.os == TargetOS::Windows)
                        src_name.replace_extension("obj");
                    else
                        src_name.replace_extension("o");
                    src_obj_map.second = std::filesystem::path("obj").append(compiledTarget->name->value().data()).append(src_name.c_str()).string();
                }
            }

            if(!graph.empty()){

            }

            graph.emplace_back(std::make_pair(t->name->value(),t));

        }

        return true;
    };

    void ExecEngine::generate(){
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


};