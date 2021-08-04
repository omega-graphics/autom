#include <iostream>
#include "ExecEngine.h"
#include "Toolchains.def"

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
        return true;
    };

    void ExecEngine::generate(){
        if(opts.gen.supportsCustomToolchainRules()){
            opts.gen.genToolchainRules(toolchain);
        };
        while(!exec->targets.empty()){
            auto t = exec->targets.front();
            if(t->type & COMPILED_OUTPUT_TARGET){
                auto * ct  = (CompiledTarget *)t;
                if(ct->source_object_map.empty()) {
                    std::cout << "ERROR: Compiled Target `" << ct->name << "` has no sources!" << std::endl;
                    break;
                }
            }
            opts.gen.consumeTarget(t);
            exec->targets.pop();
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

    
};