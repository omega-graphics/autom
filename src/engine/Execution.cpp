#include "Execution.h"
#include "../ExecEngine.h"
#include "Gen.h"

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

        void object_inc_ref(Object *obj){
            obj->refCount += 1;
        };

        void object_dec_ref(Object *obj){
            obj->refCount -= 1;
        };


        Eval::Eval(Gen &gen,ExecEngine *engine):engine(engine),gen(gen){

        };

        Object *Eval::referVarWithScope(ASTScope *scope,StrRef name){
            /// TODO: Enhance impl with multi scope eval!
            auto & first_store = vars[scope];
            auto found_it = first_store.body.find(name);
        };

        Object * Eval::evalExpr(ASTExpr *node,bool *failed){
            //  std::cout << "Eval Expr:" << "0x" << std::hex << node->type << std::dec << std::endl;
             *failed = false;
            switch(node->type){
                case EXPR_IVKE : {
                    // std::cout << "Eval Sub Expr:" << "0x" << std::hex << node->lhs->type << std::dec << std::endl;
                    if(node->lhs->type == EXPR_ID){
                        std::string & v = node->lhs->id;
                        int code;
                        Object * returnT = tryInvokeBuiltinFunc(v,node->func_args,&code);
                        if(code == INVOKE_NOTBUILTIN){
                            std::cout << "Failed to invoke function" << std::endl;
                            return nullptr;
                        }
                        else if(code == INVOKE_FAILED){
//                            std::cout << "Failed to invoke function" << std::endl;
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
                        bool f;
                        objData.push_back(evalExpr(obj,&f));
                        if(f){
                            *failed = true;
                            return nullptr;
                        };
                    };
                    return new Object {Object::Array,new Object::ArrayData {objData}};
                    break;
                }
            };
            return nullptr;
        }

        Object * Eval::evalBlock(ASTBlock *block) {
            
        }


        bool Eval::evalStmt(ASTNode *node){
            std::cout << "Eval Node" << std::endl;
            if(node->type & EXPR){
                bool f;
                evalExpr((ASTExpr *)node,&f);
                if(f){
                    return false;
                };
            }
            else {
                switch (node->type) {
                    case IMPORT_DECL : {
                        auto *decl = (ASTImportDecl *)node;
                        if(decl->isInterface){
                            /// Relative Interface File
                            if(std::filesystem::exists(decl->value)){
                                std::filesystem::path p(decl->value);
                                if(!p.has_extension())
                                    p.replace_extension(".autom");
                                importFile(p.string().c_str());
                                break;
                            }
                            else {
                                /// Interface File in Search Paths
                                for(auto & dir : engine->opts.interfaceSearchPaths){
                                    auto p = std::filesystem::path(dir.data()).append(decl->value);
                                    if(std::filesystem::exists(p)){
                                        if(!p.has_extension())
                                            p.replace_extension(".autom");
                                        importFile(p.string().c_str());
                                        break;
                                    }
                                };

                                std::cout << "Cannot import file:" << decl->value << std::endl;

                                /// Throw Error!
                                return false;
                            }
                        }
                        /// Import Interface Extension
                        else {
                            if(std::filesystem::exists(decl->value)){
                                loadExtension(decl->value);
                            }
                            else {
                                for(auto & dir : engine->opts.interfaceSearchPaths){
                                    auto p = std::filesystem::path(dir.data()).append(decl->value);
                                    if(std::filesystem::exists(p)){
                                        loadExtension(p);
                                    }
                                }

                                std::cout << "ERROR: Cannot load extension: " << decl->value << std::endl;

                                /// Throw Error!
                                return false;
                            }
                        }
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
                            bool f;
                            obj = evalExpr(decl->init.value(),&f);
                            if(f){
                                return false;
                            };
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

    Extension * eval::Eval::loadExtension(const std::filesystem::path& path){

        if(path.has_extension() && path.extension() == "aext"){

            #if !defined(DLL)
                auto data = dlopen(path.c_str(),RTLD_NOW);
                auto cb =  (AutomExtEntryFunc)dlsym(data,STR_WRAP(nativeExtMain));
                auto ext = cb();
                ext->libData = data;
            #else
                auto data = LoadLibrary((LPCSTR)path.c_str());
                auto cb = (AutomExtEntryFunc) GetProcAddress(data,STR_WRAP(nativeExtMain));
                auto ext = cb();
                ext->libData = data;
            #endif
                loadedExts.push_back(ext);
                return ext;
        }
        else {
            std::cout << "ERROR:" << "Failed to load extension: " << path << std::endl;
            return nullptr;
        }
    };

    void eval::Eval::closeExtensions(){
        for(auto ext : loadedExts){
        #if !defined(DLL)
            dlclose(ext->libData);
        #else
            FreeLibrary((HMODULE)ext->libData);
        #endif
            delete ext;
        };
    }

    void eval::Eval::importFile(const autom::StrRef & path) {
        std::ifstream in(path.data());
        auto old_value = engine->resetASTFactoryTokenIndex(0);
        auto old_vec = engine->resetASTFactoryTokenVector(nullptr);
        engine->parseAndEvaluate(&in);
        in.close();
        engine->resetASTFactoryTokenIndex(old_value);
        engine->resetASTFactoryTokenVector(old_vec);
    }
};
