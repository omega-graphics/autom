#include "../Target.h"
#include "AST.h"
#include "../ADT.h"

#ifndef AUTOM_PARSER_EXECUTION_H
#define  AUTOM_PARSER_EXECUTION_H

namespace autom {

    class Parser;

    namespace eval {

        struct Object;

        class Eval {

            Parser *parser;

            TargetConsumer &targetConsumer;

            std::vector<ASTFuncDecl *> funcs;

            struct VarStore {
                std::unordered_map<std::string,Object *> body;
            };

            std::unordered_map<ASTScope *,VarStore> vars;

            Object *referVarWithScope(ASTScope *scope,StrRef name);
            void clearVarStoreWithScope(ASTScope *scope);

            Object *invokeFunc(StrRef name);
            Object *evalBlock(ASTBlock *block);

            Object *evalExpr(ASTExpr *expr);

        public:
            bool evalStmt(ASTNode *node);
            Eval(TargetConsumer &targetConsumer,Parser *parser);

        };

    };

}

#endif