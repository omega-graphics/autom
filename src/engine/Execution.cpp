#include "Execution.h"
#include "../ExecEngine.h"
#include "Gen.h"
#include "Diagnostic.h"

#include "Builtins.def"
#include "engine/AST.def"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <iterator>
#include <sstream>



#if __has_include(<dlfcn.h>)
#include <dlfcn.h>
#elif defined(_WIN32)
#define DLL
#include <windows.h>
#endif

namespace autom {

    namespace eval {

        Eval::Eval(Gen &gen,ExecEngine *engine):engine(engine),gen(gen){

        };

        void Eval::addTarget(Target *target){
            targetCount += 1;
            targets.push_back(target);
        };

        Object *Eval::referVarWithScope(ASTScope *scope,StrRef name){
            
            auto & first_store = vars[scope];
            auto found_it = first_store.body.find(name);
            if(found_it != first_store.body.end())
                return found_it->second;
                
            for(auto & scope_var_store_p : vars){
                if(scope->isChildScopeOfParent(scope_var_store_p.first)){
                    auto _found_it = scope_var_store_p.second.body.find(name);
                    if(_found_it != scope_var_store_p.second.body.end()){
                        return _found_it->second;
                    }
                }
            }

            return nullptr;
        };

        void eval::Eval::clearVarStoreWithScope(ASTScope *scope) {
            auto found_store = vars.find(scope);
            if(found_store != vars.end()){
                vars.erase(found_store);
            }
        }

        Object * Eval::evalExpr(ASTExpr *node,bool *failed){
            //  std::cout << "Eval Expr:" << "0x" << std::hex << node->type << std::dec << std::endl;
             *failed = false;
            switch(node->type){
                case EXPR_IVKE : {
                    // std::cout << "Eval Sub Expr:" << "0x" << std::hex << node->lhs->type << std::dec << std::endl;
                    if(node->lhs->type == EXPR_ID){
                        std::string & v = node->lhs->id;
                        int code;
                        auto * returnT = tryInvokeBuiltinFunc(v,node->func_args,&code);
                        if(code == INVOKE_NOTBUILTIN){
                            
                            std::vector<std::pair<std::string,Object *>> evalParams;
                            
                            auto evalArgs = [&](std::vector<std::pair<std::string,Object *>> & args){
                                
                                for(const auto & a : node->func_args){

                                    bool f;
                                    args.emplace_back(std::make_pair(a.first,evalExpr(a.second,&f)));
                                    if(f){
                                        *failed = true;
                                        break;
                                    };
                                };
                            };

//                            /// 1. Check Functions Defined!
//
                            for(auto & func : funcs){
                                if(func->id == v){
                                    evalArgs(evalParams);
                                    return invokeFunc(func->body,evalParams);
                                };
                            };

                            /// 2. Check Extension Modules!
                            
                            for(auto & extension : loadedExts){
                                for(auto & f : extension->funcs){
                                    if(f.name == v){
                                        evalArgs(evalParams);
                                        return f.func(evalParams.size(),evalParams.data());
                                    };
                                }
                            };

                            return nullptr;
                        }
                        else if(code == INVOKE_FAILED){
                            *failed = true;
                            return nullptr;
                        };
                        return returnT;
                    }
                    break;
                }
                case EXPR_LITERAL : {
                    auto literal = (ASTLiteral *)node;
                    if(literal->isString()){
                        auto str = literal->str.value();
                        if(!processString(&str,node->scope)){
                            engine->printError("Failed to Process String!");
                            return nullptr;
                        };
                        return new eval::String(str);
                    }
                    else if(literal->isBoolean()){
                        return new eval::Boolean(literal->boolean.value());
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
                    return new eval::Array(objData);
                    break;
                }
                case EXPR_BINARY : {
                    
                    bool _failed;
                    auto lhs_obj = evalExpr(node->lhs,&_failed);
                    if(_failed){
//                        engine->printError("Failed to evaluate lhs");
                        *failed = true;
                        return nullptr;
                    }
                    
                    auto rhs_obj = evalExpr(node->rhs,&_failed);
                    
                    if(_failed){
//                        engine->printError("Failed to evaluate rhs");
                        *failed = true;
                        return nullptr;
                    }
                    
                    /// Neither side of an operator expression (Binary/Unary) can be a TargetWrapper object!
                    if(lhs_obj->type == Object::Target || rhs_obj->type == Object::Target){
                        engine->printError("Cannot perform any operators on Targets");
                        return nullptr;
                    }
                    
                    if(node->operand == OP_PLUS){
                        
                        if(lhs_obj->type == Object::Boolean || rhs_obj->type == Object::Boolean){
                            engine->printError("Cannot perform operator `+` on Booleans");
                            return nullptr;
                        }
                        
                        
                    }
                    
                    /// Success for all the above checks!
                    
                    return lhs_obj->performOperand(rhs_obj,node->operand);
                    break;
                }
                case EXPR_MEMBER : {
                    bool f;
                    auto obj = evalExpr(node->lhs,&f);
                    if(f){
                        *failed = true;
                        return nullptr;
                    }

                    autom::StrRef propName {node->id};
               
                    if(obj->type == Object::Target){

                        auto tw = (TargetWrapper *)obj;

                        if(tw->value()->type & COMPILED_OUTPUT_TARGET){
                            auto _target =  (CompiledTarget *)tw->value();
                            if(propName == "sources"){
                                return _target->srcs;
                            }
                            else if(propName == "cflags"){
                                return _target->cflags;
                            }
                            else if(propName == "include_dirs"){
                                return _target->include_dirs;
                            }
                        }
                    }
                    else {
                        std::cout << "ERROR:" << "Target and Config are the only objects that have properties!" << std::endl;
                        *failed = true;
                        return nullptr;
                    }
                    break;
                }
            };
            return nullptr;
        }

        Object * Eval::evalBlock(ASTBlock & block,const ASTBlockContext & ctxt,bool *returning) {
            for(const auto & node : block.body){
                if(ctxt.inFunction){
                    if(node->type == RETURN_DECL){
                        *returning = true;
                        auto expr = (ASTExpr *)node;
                        bool f;
                        auto * rc = evalExpr(expr->rhs,&f);
                        if(f){
                            return nullptr;
                        }
                    }
                }

            }
        }

        bool Eval::processString(std::string * str,ASTScope *scope){
            
            std::istringstream in(str->data());

            auto getChar = [&](){
                return (char)in.get();
            };

            auto aheadChar = [&](){
                char c = in.get();
                in.seekg(-1,std::ios::cur);
                return c;
            };

            std::ostringstream out;
            char c;
            while(!in.eof() && ((c = getChar()) != -1)){
                switch (c) {
                    case '$': {
                        char aheadC =aheadChar();
                        if(aheadC == '{'){
                            in.seekg(1,std::ios::cur);
                            std::string var_name = "";
                            while((c = getChar()) != '}'){
                                var_name += c;
                            }
                            auto obj = referVarWithScope(scope,var_name);
                            if(!obj){
                                engine->printError(formatmsg("Undeclared Var `@0`",var_name));
                                return false;
                            }
                            else if(objectIsString(obj)){
                                auto str = (eval::String *)obj;
                                out << str->value().data();
                            }
                            else {
                                engine->printError(formatmsg("Var `@0` is not a String!",var_name));
                                return false;
                            }
                        }
                        else {
                            out << c;
                        }
                        break;
                    }
                    default : {
                        out << c;
                        break;
                    }
                }
                
            }

            *str = out.str();


            return true;
        };


        bool Eval::evalStmt(ASTNode *node){
            // std::cout << "Eval Node" << std::endl;
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

        Object *Eval::invokeFunc(ASTBlock & block,ArrayRef<std::pair<std::string,Object *>> args) {
            bool f;
            return evalBlock(block,ASTBlockContext {true},&f);
        }

        Object * Eval::evalGenericStmt(ASTNode *node,bool *failed) {
            return nullptr;
        }
        
    }

    Extension * eval::Eval::loadExtension(const std::filesystem::path& path){

        if(path.has_extension() && path.extension() == ".aext"){

            #if !defined(DLL)
                auto data = dlopen(path.string().c_str(),RTLD_NOW);
                if(data == NULL){
                    std::cout << "Failed to load shared lib:" << path << std::endl;
                    return nullptr;
                }
            
//            #ifdef __ELF__
                AutomExtEntryFunc cb =  (AutomExtEntryFunc)dlsym(data,"nativeExtMain");
//            #else
//                AutomExtEntryFunc cb =  (AutomExtEntryFunc)dlsym(data,"_nativeExtMain");
//            #endif
            
                if(cb == NULL){
                    std::cout << "ERROR:" << dlerror() << std::endl;
                    return nullptr;
                };
                auto ext = cb();
                ext->libData = data;
            #else
                HMODULE data = LoadLibraryW(path.c_str());
                auto cb = (AutomExtEntryFunc) GetProcAddress(data,"nativeExtMain");
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
