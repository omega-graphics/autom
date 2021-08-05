

#include "Execution.h"
#include "Builtins.def"
#include "ExecEngine.h"
#include "Diagnostic.h"
#include <initializer_list>
#include <iostream>

namespace autom {

    template<>
    struct Format<eval::Object::Ty> {
        static void format(std::ostream & out,eval::Object::Ty & val){
            switch (val) {
                case eval::Object::String : {
                    out << "String";
                    break;
                }
                case eval::Object::Boolean : {
                    out << "Boolean";
                    break;
                }
                case eval::Object::Array : {
                    out << "Array";
                    break;
                }
                case eval::Object::Target : {
                    out << "Target";
                    break;
                }
            }
        }
    };

}

namespace autom::eval {

    void object_inc_ref(Object *obj);
    void object_dec_ref(Object *obj);

        #define STRING_OBJECT(o) ((Object::StrData *)(o)->data)->data
        #define ARRAY_OBJECT(o) ((Object::ArrayData *)(o)->data)->data
        #define TARGET_OBJECT(o) (Target *)(o)->data)

        #define STRING_ARRAY_OBJECT(ob,v)\
        auto & s = ARRAY_OBJECT(ob);\
        std::vector<std::string> (v);\
        for(auto & _o : s){\
        TYPECHECK_STRICT(_o,Object::String)\
        (v).push_back(STRING_OBJECT(_o));\
        \
        }

    inline void _print_to_stream(std::ostream & out,Object *obj){
        switch (obj->type) {
            case Object::String : {
                out << "\x1b[33m" << std::quoted(castToString(obj)->value().data()) << "\x1b[0m" << std::endl;
                break;
            }
            case Object::Array : {
                auto * array = castToArray(obj);
                out << "[";
                for(unsigned i = 0;i < array->length();i++){
                    if(i != 0){
                        out << ",";
                    };
                    _print_to_stream(out,array->getBeginIterator()[i]);
                };
                out << "]";
            }
        }
    };

    struct EvalContext {
        Eval *eval;
        ExecEngine *execEngine;
        int code;
        inline void logError(const std::string & msg){
            execEngine->printError(msg);
        };
        inline void setCode(int c){
            code = c;
        };
    };


    Object *bf_print(MapRef<std::string,Object *> args,EvalContext & ctxt){
        _print_to_stream(std::cout,args["msg"]);
        return nullptr;
    };



    Object *bf_Project(MapRef<std::string,Object *> args,EvalContext & ctxt){
        
        auto *name = castToString(args["name"]);
        auto *version = castToString(args["version"]);

        std::cout << "Configuring Project " << name->value().data() << " " << version->value().data() << std::endl;

        return nullptr;
    };

    Object *bf_Executable(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);

        auto t = CompiledTarget::Executable(name,srcs);

         ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };

    Object *bf_Archive(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);
        auto t = CompiledTarget::Archive(name,srcs);

        ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };

    Object *bf_Shared(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);
        auto t = CompiledTarget::Shared(name,srcs);

        ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };



    Object * Eval::tryInvokeBuiltinFunc(autom::StrRef subject,std::unordered_map<std::string,ASTExpr *> & args,int * code){

        *code = INVOKE_SUCCESS;

        std::vector<std::pair<std::string,Object *>> ready_args;

        auto checkArgs = [&](std::initializer_list<std::pair<CString,Object::Ty>> params){
            #define TYPECHECK_STRICT(object,t) if((t) != Object::Any && (object)->type != (t)){ engine->printError(formatmsg("Param `@0` is not a typeof @1",autom::StrRef(p.first),t));return false; }
            bool rc = true;
            for(auto & p : params){
                auto found_it = args.find(p.first);
                if(found_it == args.end()){
                    engine->printError(formatmsg("Function `@0` does not take argument named `@1`",subject,p.first));
                    rc = false;
                    break;
                }
                else {
                    bool f;
                    auto obj = evalExpr(found_it->second,&f);
                    if(f){
                        rc = false;
                        break;
                    }
                    TYPECHECK_STRICT(obj,p.second);
                    ready_args.push_back(std::make_pair(p.first,obj));
                };
            };
            return rc;
            #undef TYPECHECK_STRICT
        };

        EvalContext ctxt {this,engine,0};

        #define BUILTIN_FUNC(name,func,...)if(subject == name){\
            if(checkArgs({__VA_ARGS__})){\
                return func({ready_args.data(),ready_args.data() + ready_args.size()},ctxt);\
            }\
            else {\
                *code = INVOKE_FAILED;\
                return nullptr;\
            };\
        }
        
        BUILTIN_FUNC(BUILTIN_PRINT,bf_print,{"msg",Object::Any});    

        BUILTIN_FUNC(BUILTIN_PROJECT,bf_Project,{"name",Object::String},{"version",Object::String});
         
        BUILTIN_FUNC(BUILTIN_EXECUTABLE,bf_Executable,{"name",Object::String},{"sources",Object::Array});

        BUILTIN_FUNC(BUILTIN_ARCHIVE,bf_Archive,{"name",Object::String},{"sources",Object::Array});

        BUILTIN_FUNC(BUILTIN_SHARED,bf_Shared,{"name",Object::String},{"sources",Object::Array});

        
        *code = INVOKE_NOTBUILTIN;
        return nullptr;
    };
}

