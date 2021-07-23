#include "Parser.h"

namespace autom {
    Parser::Parser(TargetConsumer &targetConsumer):
    lexer(std::make_unique<Lexer>()),
    astFactory(std::make_unique<ASTFactory>(*lexer)),
    exec(std::make_unique<eval::Eval>(targetConsumer,this)){
       
    };

    void Parser::parseAndEvaluate(std::istream * in){
        std::vector<Tok> tokenVector;

        lexer->setInputStream(in);
        lexer->tokenize(&tokenVector);   
        lexer->finish();
        
        astFactory->setTokenVector(&tokenVector);   

        ASTNode *node;
        while((node = astFactory->nextStmt()) != nullptr){
            exec->evalStmt(node);
        };
      
 
        
    };

    Parser::~Parser(){
        lexer->finish();
    };

    
};