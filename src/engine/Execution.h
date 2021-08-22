
#include "AST.h"
#include "../ADT.h"
#include "Autom.h"

#include <filesystem>
#include <deque>


#ifndef AUTOM_ENGINE_EXECUTION_H
#define  AUTOM_ENGINE_EXECUTION_H

namespace autom {

    struct Target;
    
    class ExecEngine;
    class Gen;

    namespace eval {
        /// @brief A Runtime Object
        struct Object {
            typedef enum : int {
                Target, // data = Target *
                String, // data = StrData *
                Array, // data = ArrayData *
                Boolean, // data = BoolData *,
                Any = (Target | String | Array | Boolean)
            } Ty;
            Ty type;
            unsigned refCount = 1;

            /// @brief Increment object reference count
            void incRef();

            /// @brief Decrement object reference count
            void decRec();
            Object(Ty type);
            virtual ~Object(){};
        };

        class Boolean : public Object {
            bool data;
        public:
            bool value();
            void assign(Boolean *other);
            Boolean(bool & val);
            ~Boolean();
        };

        class String : public Object {
            std::string data;
        public:
            size_t length();
            bool empty();
            StrRef value() const;
            void assign(String *other);
            void assign(const std::string& string);
            String();
            String(std::string & val);
            ~String();
        };

        class Array : public Object {
            std::vector<Object *> data;
        public:
            typedef std::vector<Object *>::iterator Iterator;
            Iterator getBeginIterator();
            Iterator getEndIterator(); 
            size_t length();
            bool empty();

            void assign(Array *other);
            
            ArrayRef<Object *> value()const;
            std::vector<std::string> toStringVector();

            template<Object::Ty _t>
            bool isArrayOf(){
                for(auto it = getBeginIterator();it != getEndIterator();it++){
                    if((*it)->type != _t){
                        return false;
                    };
                };
                return true;
            };
            explicit Array();
            Array(std::vector<Object *> & val);
            ~Array();
        };


        class TargetWrapper : public Object {
            ::autom::Target *t;
        public:
            TargetWrapper(::autom::Target *_t):Object(Object::Target),t(_t){};
            ::autom::Target *value() const;
            ~TargetWrapper();
        };

        Boolean * castToBool(Object *object);

        String * castToString(Object *object);

        Array * castToArray(Object *object);




        class Eval {
            friend class ::autom::ExecEngine;
            std::deque<Target *> targets;

            std::vector<Extension *> loadedExts;

            ExecEngine *engine;

            Gen &gen;

            std::vector<ASTFuncDecl *> funcs;

            Object * tryInvokeBuiltinFunc(autom::StrRef subject,std::unordered_map<std::string,ASTExpr *> & args,int * code);

        public:
            struct VarStore {
                std::unordered_map<std::string,Object *> body;
            };
            std::unordered_map<ASTScope *,VarStore> vars;
            
            Object *referVarWithScope(ASTScope *scope,StrRef name);

            void clearVarStoreWithScope(ASTScope *scope);

            void addTarget(Target *target);

        private:
            bool processString(std::string * str,ASTScope *scope);





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