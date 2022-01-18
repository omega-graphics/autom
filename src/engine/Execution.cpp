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
    
        void Eval::VarStore::deallocAll(){
            for(auto & ent : body){
                if(ent.second != nullptr){
//                    delete ent.second;
                }
            };
            body.clear();
        }
        
        Eval::VarStore::~VarStore(){
            deallocAll();
        }

        Eval::Eval(Gen &gen,ExecEngine *engine):engine(engine),gen(gen),currentEvalDir("."),currentGenContext(nullptr){
            /// Current Eval Dir should be current directory of execution
        };

        void Eval::addTarget(Target *target){
            projects[*currentGenContext].emplace_back(target);
            totalTargets += 1;
        };
    
        void Eval::setGlobalVar(autom::StrRef str,Object *object){
            vars[GLOBAL_SCOPE].body.insert(std::make_pair(str,object));
        };

        Object *Eval::referVarWithScope(ASTScope *scope,StrRef name){
            if(scope->parent != nullptr){
                std::cout << formatmsg("Current Scope: @0, Parent Scope: @2, Var: @1",scope->name,name,scope->parent->name).res << std::endl;
            }
            else {
                std::cout << formatmsg("Current Scope: @0, Var: @1",scope->name,name).res << std::endl;
            }
            
            auto first_store_it = vars.find(scope);
            if(first_store_it != vars.end()){
                auto & first_store = first_store_it->second;
                
                auto found_it = first_store.body.find(name);
                if(found_it != first_store.body.end())
                    return found_it->second;
            }
                
            for(auto & scope_var_store_p : vars){
                if(scope->isChildScopeOfParent(scope_var_store_p.first)){
                    auto _found_it = scope_var_store_p.second.body.find(name);
                    if(_found_it != scope_var_store_p.second.body.end()){
                        return _found_it->second;
                    }
                }
            }
            
            engine->printError(formatmsg("Failed to reference variable `@0`",name));

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
                                        engine->printError(formatmsg("Failed to evaluate func arg for @0",v));
                                        *failed = true;
                                        return nullptr;
                                    };
                                };
                            };

//                            /// 1. Check Functions Defined!
//
                            for(auto & func : funcs){
                                if(func->id == v){
                                    evalArgs(evalParams);
                                    return invokeFunc(func->body.get(),evalParams);
                                };
                            };

                            /// 2. Check Extension Modules!
                            
                            for(auto & extension : loadedExts){
                                for(auto & f : extension->funcs){
                                    if(v == f.name){
                                        evalArgs(evalParams);
                                        return f.func(evalParams.size(),evalParams.data());
                                    };
                                }
                            };
                            
                            *failed = true;
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
//                        if(!processString(&str,node->scope)){
//                            engine->printError("Failed to Process String!");
//                            *failed = true;
//                            return nullptr;
//                        };
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
                    
                    /// TODO: Implement binary operand checks!
                    
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
                case EXPR_ASSIGN : {
                    bool f;
                    auto lhs = evalExpr(node->lhs,&f);
                    if(f){
                        *failed = true;
                        return nullptr;
                    }
                    
                    auto rhs = evalExpr(node->rhs,&f);
                    if(f){
                        *failed = true;
                        return nullptr;
                    }
                    
//                    std::cout << "LHS:" << (int)lhs->type << std::endl;
//                    std::cout << "RHS:" << (int)rhs->type << std::endl;
                    
                    switch (lhs->type) {
                        case Object::String: {
                            assert(objectIsString(rhs));
                            castToString(lhs)->assign(castToString(rhs));
                            break;
                        }
                        case Object::Array : {
                            assert(objectIsArray(rhs));
                            castToArray(lhs)->assign(castToArray(rhs));
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    
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
                        
                        if(propName == "deps"){
                            return tw->value()->deps;
                        }

                        if(tw->value()->type & COMPILED_OUTPUT_TARGET){
                            auto _target =  (CompiledTarget *)tw->value();
                          
                            if(propName == "cflags"){
                                return _target->cflags;
                            }
                            else if(propName == "output_dir"){
                                return _target->output_dir;
                            }
                            else if(propName == "include_dirs"){
                                return _target->include_dirs;
                            }
                            else if(propName == "output_ext"){
                                return _target->output_ext;
                            }
                            else if(propName == "libs"){
                                return _target->libs;
                            }
                            else if(propName == "lib_dirs"){
                                return _target->lib_dirs;
                            }
#ifdef __APPLE__
                            else if(propName == "frameworks"){
                                return _target->frameworks;
                            }
                            else if(propName == "framework_dirs"){
                                return _target->framework_dirs;
                            }
#else
                            else if(propName == "frameworks" || propName == "framework_dirs"){
                                return nullptr;
                            }
#endif
                        }
                        else if(tw->value()->type == SCRIPT_TARGET){
                            auto _target =  (ScriptTarget *)tw->value();
                          
                            if(propName == "desc"){
                                return _target->desc;
                            }
                        }
                        else if(tw->value()->type & JAVA_TARGET){
                            auto _target = (JavaTarget *)tw->value();
                            if(propName == "maven_repos"){
                                return _target->maven_repos;
                            }
                            else if(propName == "maven_deps"){
                                return _target->maven_deps;
                            }
                        }
                    }
                    else if(obj->type == Object::Namespace){
                        auto ns = (Namespace *)obj;
                        
                        auto obj = ns->get(propName);
                        if(obj == nullptr){
                            *failed = true;
                            engine->printError(formatmsg("Property `@0` does not exist on Scope object",propName));
                            return nullptr;
                        }
                        return obj;
                    }
                    else {
                        engine->printError("Targets and Scopes are the only objects that have properties!");
                        *failed = true;
                        return nullptr;
                    }
                    break;
                }
            };
            return nullptr;
        }

        Object * Eval::evalBlock(ASTBlock * block,const ASTBlockContext & ctxt,bool *failed,bool *returning) {
            *failed = false;
            *returning = false;
            for(const auto & node : block->body){
                if(ctxt.inFunction){
                    if(node->type == RETURN_DECL){
                        *returning = true;
                        auto expr = (ASTExpr *)node;
                        bool f;
                        auto * rc = evalExpr(expr->rhs,&f);
                        if(f){
                            return nullptr;
                        }
                        return rc;
                    }
                }
                bool f;
                auto o = evalGenericStmt(node,&f,true,returning);
                
                if(f){
                    *failed = true;
                    return nullptr;
                };
                
                if(ctxt.inFunction && (*returning)){
                    return o;
                }
                

            }
            return nullptr;
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
            bool f;
            evalGenericStmt(node,&f);
            return !f;
        };

        Object *Eval::invokeFunc(ASTBlock * block,ArrayRef<std::pair<std::string,Object *>> args) {
            bool f,r;
            VarStore store {};
            for(auto & a : args){
                std::cout << (int)a.second->type << std::endl;
                store.body.insert(a);
            }
            vars.insert(std::make_pair(block->scope,store));
            auto obj = evalBlock(block,ASTBlockContext {true},&f,&r);
            auto it = vars.find(block->scope);
            vars.erase(it);
            if(f){
                engine->printError("Failed to evaluate func..");
                return nullptr;
            }
            return obj;
        }

        Object * Eval::evalGenericStmt(ASTNode *node,bool *failed,bool inFunctionCtxt,bool *returning) {
            *failed = false;
            
//             std::cout << "Eval Node:" << std::hex << int(node->type) << std::dec << std::endl;
            if(node->type & EXPR){
                bool f;
                auto obj = evalExpr((ASTExpr *)node,&f);
                if(f){
                    return nullptr;
                };
                return obj;
            }
            else {
                switch (node->type) {
                    case IMPORT_DECL : {
                        if(node->scope != GLOBAL_SCOPE){
                            *failed = true;
                            return nullptr;
                        }
                        auto *decl = (ASTImportDecl *)node;
                        if(decl->isInterface){
                            /// Relative Interface File
                            std::filesystem::path p(decl->value);
                            if(!p.has_extension())
                                p.replace_extension(".autom");
                            if(std::filesystem::exists(p)){
                               
                                auto prior_eval_dir = currentEvalDir;
                                currentEvalDir = p.parent_path();
                                importFile(p.string().c_str());
                                currentEvalDir = prior_eval_dir;
                                break;
                            }
                            else {
                                bool success = false;
                                /// Interface File in Search Paths
                                for(auto & dir : engine->opts.interfaceSearchPaths){
                                    auto p = std::filesystem::path(dir.data()).append(decl->value);
                                    
                                    if(!p.has_extension())
                                        p.replace_extension(".autom");
                                    
                                    if(std::filesystem::exists(p)){
                                        auto prior_eval_dir = currentEvalDir;
                                        currentEvalDir = p.parent_path();
                                        importFile(p.string().c_str());
                                        currentEvalDir = prior_eval_dir;
                                        success = true;
                                        break;
                                    }
                                };
                                
                                if(success){
                                    break;
                                }

                                engine->printError(formatmsg("Cannot import file: @0",decl->value));

                                /// Throw Error!
                                *failed = true;
                                return nullptr;
                            }
                        }
                        /// Import Interface Extension
                        else {
                            if(std::filesystem::exists(decl->value)){
                                loadExtension(decl->value);
                            }
                            else {
                                bool success = false;
                                for(auto & dir : engine->opts.interfaceSearchPaths){
                                    auto p = std::filesystem::path(dir.data()).append(decl->value);
                                    if(std::filesystem::exists(p)){
                                        loadExtension(p);
                                        success = true;
                                    }
                                }
                                
                                if(success){
                                    break;
                                }

                                engine->printError(formatmsg(" Cannot load extension: @0",decl->value));

                                /// Throw Error!
                                *failed = true;
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case FUNC_DECL : {
                        funcs.push_back((std::shared_ptr<ASTFuncDecl>)(ASTFuncDecl *)node);
                        break;
                    }
                    case VAR_DECL : {
                        auto *decl = (ASTVarDecl *)node;
                        Object *obj;
                        if(decl->init.has_value()){
                            bool f;
                            obj = evalExpr(decl->init.value(),&f);
                            if(f){
                                *failed = true;
                                return nullptr;
                            };
                        }
                        else {
                            obj = nullptr;
                        };
                        vars[node->scope].body.insert(std::make_pair(decl->id,obj));
                        break;
                    }
                    case COND_DECL : {
                        auto *decl = (ASTConditionalDecl *)node;
                        
                        for(auto case_it = decl->cases.begin();case_it != decl->cases.end();case_it++){
                            auto &_case = *case_it;
                            
                            bool f;
                            
                            if(_case.testCondition){
                                /// 1. Eval Expression
                                eval::Boolean * obj = (eval::Boolean *)evalExpr(_case.condition,&f);
                                if(f){
                                    *failed = true;
                                    return nullptr;
                                }
                                
                                /// 2 . Assert that the evaluated expression returned a Boolean object.
                                
                                if(obj->type != Object::Boolean){
                                    *failed = true;
                                    engine->printError("Object evaluated in conditional test case is not a Boolean.");
                                    return nullptr;
                                }
                                
                                /// 3. Test condition
                                
                                if(obj->value()){
                                    ASTBlockContext ctxt {inFunctionCtxt};
                                    bool ret = false;
                                    auto obj = evalBlock(_case.block.get(),ctxt,&f,&ret);
                                    if(f){
                                        *failed = true;
                                        return nullptr;
                                    }
                                    
                                    if(inFunctionCtxt && ret){
                                        return obj;
                                    }
                                    
                                    break;
                                }
                                else {
                                    /// Goto to next case if there is another.
                                    continue;
                                }
                                
                                
                            }
                            else {
                            
                                ASTBlockContext ctxt {inFunctionCtxt};
                                
                                
                                bool ret = false;
                                auto obj = evalBlock(_case.block.get(),ctxt,&f,&ret);
                                if(f){
                                    *failed = true;
                                    return nullptr;
                                }
                                
                                if(inFunctionCtxt && ret){
                                    return obj;
                                }
                            }
                        }
                        break;
                    }
                    case FOREACH_DECL : {
                        auto *decl = (ASTForeachDecl *)node;
                        
                        auto & varStoreEntry = vars[decl->body->scope];
                        
                        varStoreEntry.body.insert(std::make_pair(decl->loopVar,nullptr));
                        bool f;
                        
                        eval::Array * listObject = (eval::Array * )evalExpr(decl->list,&f);
                        if(f){
                            *failed = true;
                            return nullptr;
                        }
                        
                        if(listObject->type != Object::Array){
                            *failed = true;
                            engine->printError("Object in foreach context must be an Array!");
                            return nullptr;
                        }
                        
                        auto length = listObject->length();
                        auto begin_it = listObject->getBeginIterator();
                        
                        ASTBlockContext ctxt {inFunctionCtxt};
                        
                        bool ret = false;
                        
                        while(length > 0){
                            varStoreEntry.body[decl->loopVar] = *begin_it;
                            auto obj = evalBlock(decl->body.get(),ctxt,&f,&ret);
                            if(f){
                                *failed = true;
                                return nullptr;
                            }
                            /// Erase All Objects
                            varStoreEntry.deallocAll();
                            
                            if(inFunctionCtxt && ret){
                                return obj;
                            }
                            --length;
                            ++begin_it;
                        };
                        
                        break;
                    }
                    
                }
            };
            
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
