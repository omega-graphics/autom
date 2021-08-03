#include "ExecEngine.h"

namespace autom {
    ExecEngine::ExecEngine(ExecEngineOpts &opts) :
    opts(opts),
    lexer(std::make_unique<Lexer>()),
    astFactory(std::make_unique<ASTFactory>(*lexer)),
    exec(std::make_unique<eval::Eval>(opts.gen,this)){
       
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
            opts.gen.genToolchainRules();
        };
        while(!exec->targets.empty()){
            opts.gen.consumeTarget(exec->targets.front());
            exec->targets.pop();
        };
        opts.gen.finish();
    };

    ExecEngine::~ExecEngine(){
        lexer->finish();
    };

    
};