#include "ExecEngine.h"

namespace autom {
    ExecEngine::ExecEngine(TargetConsumer &targetConsumer,ExecEngineOpts & opts):
    opts(opts),
    lexer(std::make_unique<Lexer>()),
    astFactory(std::make_unique<ASTFactory>(*lexer)),
    exec(std::make_unique<eval::Eval>(targetConsumer,this)){
       
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

    ExecEngine::~ExecEngine(){
        lexer->finish();
    };

    
};