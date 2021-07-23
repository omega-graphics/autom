#include "AST.h"

#ifndef AUTOM_PARSER_ASTFACTORY_H
#define  AUTOM_PARSER_ASTFACTORY_H

namespace autom {

    class Lexer;

    struct Tok;
    /**
     Constructs AST for Evaluation Engine.
    */
    class ASTFactory {
        Lexer & lexer;

        unsigned privTokIndex;

        std::vector<Tok> *tokStream;

        Tok & nextToken();

        Tok & aheadToken();
        
        ASTNode *buildDecl(Tok & first_tok,ASTScope *scope);

        ASTNode *buildExpr(Tok & first_tok,ASTScope *scope);
    public:
        void setTokenVector(std::vector<Tok> *tokStream);
        ASTNode * nextStmt();
        ASTFactory(Lexer & lexer);
    };

};

#endif