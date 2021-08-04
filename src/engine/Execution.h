#include "../Target.h"
#include "AST.h"
#include "../ADT.h"
#include "Autom.h"

#include <filesystem>
#include <queue>


#ifndef AUTOM_ENGINE_EXECUTION_H
#define  AUTOM_ENGINE_EXECUTION_H

namespace autom {

    class ExecEngine;
    class Gen;

    namespace eval {

        struct Object {
            typedef enum : int {
                Target, // data = Target *
                String, // data = StrData *
                Array, // data = ArrayData *
                Boolean, // data = BoolData *,
                Any = (Target | String | Array | Boolean)
            } Ty;
            Ty type;
            void *data = nullptr;

            struct BoolData {
                bool data;
            };

            struct StrData {
                std::string data;
            };

            struct ArrayData {
                std::vector<Object *> data;
            };
            unsigned refCount = 1;

        };

        class Eval {
            friend class ::autom::ExecEngine;
            std::queue<Target *> targets;

            std::vector<Extension *> loadedExts;

            ExecEngine *engine;

            Gen &gen;

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

            Object *evalExpr(ASTExpr *expr,bool *failed);

            void importFile(const autom::StrRef & path);

            Extension *loadExtension(const std::filesystem::path& path);
            void closeExtensions();
        public:
            bool evalStmt(ASTNode *node);
            Eval(Gen &gen,ExecEngine *engine);

        };

    };

}

#endif