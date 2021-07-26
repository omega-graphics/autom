#include "Execution.h"
#include "../ExecEngine.h"

#include "Builtins.def"

#include <filesystem>
#include <fstream>
#include <iostream>

#if __has_include(<dlfcn.h>)
#include <dlfcn.h>
#elif defined(_WIN32)
#define DLL
#include <windows.h>
#endif

namespace autom {

    namespace eval {

        struct Object {
            typedef enum : int {
                Target, // data = Target *
                String, // data = StrData * 
                Array, // data = ArrayData *
                Boolean // data = BoolData *
            } Ty;
            Ty type;
            void *data;

            struct BoolData {
                bool data;
            };

            struct StrData {
                std::string data;
            };

            struct ArrayData {
                std::vector<Object *> data;
            };

        };

        #define INVOKE_FAILED -0x01
        #define INVOKE_SUCCESS 0x00
        #define INVOKE_NOTBUILTIN 0x01

        

        Object * Eval::tryInvokeBuiltinFunc(autom::StrRef subject,std::unordered_map<std::string,ASTExpr *> & args,int * code){
            #define ERROR_RETURN \
            *code = INVOKE_FAILED;\
             return nullptr;

             *code = INVOKE_SUCCESS;

            #define TYPECHECK_STRICT(object,t) if(object->type != t){ ERROR_RETURN }

            #define BUILTIN_FUNC(name,...)\
            if(subject == name) {\
                std::vector<std::pair<autom::StrRef,Object::Ty>> _args = {__VA_ARGS__};\
                if(_args.size() != args.size()){ \
                /* Incorrect Number of Args*/ \
                ERROR_RETURN };\
                \
                std::unordered_map<std::string,Object *> funcArgs;\
                for(auto & a : _args){\
                    auto it = args.find(a.first);\
                    if(it != args.end()){ ERROR_RETURN; break;}\
                    else { auto expr = evalExpr(it->second);\
                    TYPECHECK_STRICT(expr,a.second)\
                    funcArgs.insert(std::make_pair(a.first,expr));}\
                }\
                {\


            #define BUILTIN_FUNC_END(name) }} // end name

            #define STRING_OBJECT(o) ((Object::StrData *)o->data)->data
            #define ARRAY_OBJECT(o) ((Object::ArrayData *)o->data)->data
            #define TARGET_OBJECT(o) (Target *)o->data)

            #define STRING_ARRAY_OBJECT(ob,v)\
            auto & s = ARRAY_OBJECT(ob);\
            std::vector<std::string> v;\
            for(auto & _o : s){\
                TYPECHECK_STRICT(_o,Object::String)\
                v.push_back(STRING_OBJECT(_o));\
            \
            }

            #define VOID_RETURN return nullptr;

            /**
            @brief Declares the Project
             func Project(name,version) -> void
            */

            BUILTIN_FUNC(BUILTIN_PROJECT,{"name",Object::String},{"version",Object::String})
                
                auto & n = STRING_OBJECT(funcArgs["name"]);
                auto & v = STRING_OBJECT(funcArgs["version"]);

                std::cout << "Configuring Project:" << n << " " << v << std::endl;

                VOID_RETURN

            BUILTIN_FUNC_END(BUILTIN_PROJECT)

             /**
             @brief Defines an Executable Target to be Compiled
             func Executable(name,sources) -> Executable
            */

            BUILTIN_FUNC(BUILTIN_EXECUTABLE,{"name",Object::String},{"sources",Object::Array})

                auto & n = STRING_OBJECT(funcArgs["name"]);
                STRING_ARRAY_OBJECT(funcArgs["sources"],srcs_array);
                
                return new Object{Object::Target,CompiledTarget::Executable(n,srcs_array)};

            BUILTIN_FUNC_END(BUILTIN_EXECUTABLE)

            *code = INVOKE_NOTBUILTIN;
            return nullptr;
        };


        Eval::Eval(TargetConsumer &targetConsumer,ExecEngine *engine):engine(engine),targetConsumer(targetConsumer){

        };

        Object *Eval::referVarWithScope(ASTScope *scope,StrRef name){
            /// TODO: Enhance impl with multi scope eval!
            auto & first_store = vars[scope];
            return first_store.body[name];
        };

        Object * Eval::evalExpr(ASTExpr *node){
            switch(node->type){
                case EXPR_IVKE : {
                    if(node->lhs->type == EXPR_ID){
                        std::string & v = node->lhs->id;
                        int code;
                        Object * returnT = tryInvokeBuiltinFunc(v,node->func_args,&code);
                        if(code == INVOKE_NOTBUILTIN){
                            
                        }
                        else if(code == INVOKE_FAILED){
                            return nullptr;
                        };
                        return returnT;
                    }
                    break;
                }
                case EXPR_LITERAL : {
                    auto literal = (ASTLiteral *)node;
                    if(literal->isString()){
                        return new Object {Object::String,new Object::StrData {literal->str.value()}};
                    }
                    else if(literal->isBoolean()){
                        return new Object {Object::Boolean,new Object::BoolData {literal->boolean.value()}};
                    };
                    break;
                }
                case EXPR_ID : {
                    return referVarWithScope(node->scope,node->id);
                    break;
                }
                case EXPR_ARRAY : {
                    std::vector<Object *> objData;
                    for(auto obj : node->children){
                        objData.push_back(evalExpr(obj));
                    };
                    return new Object {Object::Array,new Object::ArrayData {objData}};
                    break;
                }
            };
            return nullptr;
        };

        bool Eval::evalStmt(ASTNode *node){
            if(node->type & EXPR){
                auto e = evalExpr((ASTExpr *)node);
                if(!e){
                    return false;
                };
            }
            else {
                switch (node->type) {
                    case IMPORT_DECL : {
                    auto *decl = (ASTImportDecl *)node;
                    /// Relative I File
                    if(std::filesystem::exists(decl->value)){
                        std::ifstream in(decl->value);
                        engine->parseAndEvaluate(&in);
                    }
                    else {
                        /// I File in Search Paths
                        
                        /// Throw Error!
                            return false;
                    };
                    break;
                    }
                    case FUNC_DECL : {
                        funcs.push_back((ASTFuncDecl *)node);
                        break;
                    }
                    case VAR_DECL : {
                        auto *decl = (ASTVarDecl *)node;
                        Object *obj;
                        if(decl->init.has_value()){
                            obj = evalExpr(decl->init.value());
                        }
                        else {
                            obj = nullptr;
                        };
                        vars[node->scope].body.insert(std::make_pair(decl->id,obj));
                        break;
                    }
                }
            };
            return true;
        };
        
    }

    typedef eval::Object Object;

    bool objectIsBool(Object *object){
        return object->type == Object::Boolean;
    };

    bool objectIsString(Object *object){
        return object->type == Object::String;
    };

    bool objectIsArray(Object *object){
        return object->type == Object::Array;
    };

    Object *toObject(bool & val){
        return new Object {Object::Boolean,new Object::BoolData {val}};
    };

    Object *toObject(std::string &val){
        return new Object {Object::String,new Object::StrData {val}};
    };

    Object *toObject(std::vector<Object *> &val){
        return new Object {Object::String,new Object::ArrayData {val}};
    };


    bool & objectToBool(Object *object){
        return ((Object::BoolData *)object->data)->data;
    };

    std::string & objectToString(Object *object){
        return ((Object::StrData *)object->data)->data;
    };

    std::vector<Object *> objectToVector(Object *object){
        return ((Object::ArrayData *)object->data)->data;
    };

    Extension * eval::Eval::loadExtension(std::filesystem::path path){

        #ifdef _DLFCN_H_
        auto data = dlopen(path.c_str(),RTLD_NOW);
        auto cb =  (AutomExtEntryFunc)dlsym(data,STR_WRAP(nativeExtMain));
        auto ext = cb();
        ext->libData = data;
        #else 

        #endif
        loadedExts.push_back(ext);
        return ext;
    };

    void eval::Eval::closeExtensions(){
        for(auto ext : loadedExts){
            dlclose(ext->libData);
            delete ext;
        };
    };

}