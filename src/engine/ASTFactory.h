#include "AST.h"

#ifndef AUTOM_ENGINE_ASTFACTORY_H
#define  AUTOM_ENGINE_ASTFACTORY_H

namespace autom {

    class Lexer;
    class ExecEngine;

    struct Tok;
    /**
     Constructs AST for Evaluation Engine.
    */
    class ASTFactory {
        friend class ExecEngine;
        Lexer & lexer;

        unsigned privTokIndex;

        std::vector<Tok> *tokStream;

        Tok & nextToken();

        Tok & aheadToken();

        Tok & currentToken();

        void incToNextToken();
        
        ASTBlock *buildBlock(Tok & first_tok,ASTScope *scope);
        
        ASTNode *buildDecl(Tok & first_tok,ASTScope *scope);

        ASTExpr *buildExpr(Tok & first_tok,ASTScope *scope);
        /// @name Expression Builder Funcs
        /// @{
        ASTExpr *evalObjExpr(Tok &first_tok,ASTScope *scope);
        ASTExpr *evalArgsExpr(Tok &first_tok,ASTScope *scope);
        ASTExpr *evalOpExpr(Tok &first_tok,ASTExpr *lhs,ASTScope *scope);
        /// @}
        ExecEngine *engine;
    public:
        void setTokenVector(std::vector<Tok> *tokStream);
        ASTNode * nextStmt();
        ASTFactory(Lexer & lexer);
    };

};

#endif
