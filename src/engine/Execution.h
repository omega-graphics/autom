#include "../Target.h"
#include "AST.h"
#include "../ADT.h"
#include "Autom.h"

#include <filesystem>


#ifndef AUTOM_ENGINE_EXECUTION_H
#define  AUTOM_ENGINE_EXECUTION_H

namespace autom {

    class ExecEngine;

    namespace eval {

        struct Object;

        class Eval {

            std::vector<Extension *> loadedExts;

            ExecEngine *engine;

            TargetConsumer &targetConsumer;

            std::vector<ASTFuncDecl *> funcs;

            Object * tryInvokeBuiltinFunc(autom::StrRef subject,std::unordered_map<std::string,ASTExpr *> & args,int * code);

            struct VarStore {
                std::unordered_map<std::string,Object *> body;
            };

            std::unordered_map<ASTScope *,VarStore> vars;

            Object *referVarWithScope(ASTScope *scope,StrRef name);
            void clearVarStoreWithScope(ASTScope *scope);

            Object *invokeFunc(StrRef name);
            Object *evalBlock(ASTBlock *block);

            Object *evalExpr(ASTExpr *expr);

            Extension *loadExtension(std::filesystem::path path);
            void closeExtensions();
        public:
            bool evalStmt(ASTNode *node);
            Eval(TargetConsumer &targetConsumer,ExecEngine *engine);

        };

    };

}

#endif