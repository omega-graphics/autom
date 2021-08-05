

#include "Execution.h"
#include "Builtins.def"
#include "ExecEngine.h"
#include <iostream>

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
                out << "\x1b[33m" << std::quoted(STRING_OBJECT(obj)) << "\x1b[0m" << std::endl;
                break;
            }
            case Object::Array : {
                auto & array = ARRAY_OBJECT(obj);
                out << "[";
                for(unsigned i = 0;i < array.size();i++){
                    if(i != 0){
                        out << ",";
                    };
                    _print_to_stream(out,array[i]);
                };
                out << "]";
            }
        }
    };



    Object * Eval::tryInvokeBuiltinFunc(autom::StrRef subject,std::unordered_map<std::string,ASTExpr *> & args,int * code){
            #define ERROR_RETURN \
            *code = INVOKE_FAILED;\
            return nullptr;

        *code = INVOKE_SUCCESS;

            #define TYPECHECK_STRICT(object,t) if((t) != Object::Any && (object)->type != (t)){ std::cout << "TYPECHECK FAILED!" << std::endl; ERROR_RETURN }

            #define BUILTIN_FUNC(name,...)\
            if(subject == name) {\
            std::vector<std::pair<autom::StrRef,Object::Ty>> _args = {__VA_ARGS__};\
            for(auto & a : args){ /*std::cout << "ARG:" << a.first << std::endl;*/}\
            if(_args.size() != args.size()){ \
            /* Incorrect Number of Args*/ \
            ERROR_RETURN };\
            \
            std::unordered_map<std::string,Object *> funcArgs;\
            for(auto & a : _args){\
            /*std::cout << "ARG:" << a.first.data() << std::endl;*/\
            auto it = args.find(a.first);\
            if(it == args.end()){ /*std::cout << "ARG NOT FOUND!" << std::endl;*/ ERROR_RETURN; break;}\
            else { bool failed;\
            auto expr = evalExpr(it->second,&failed);\
            if(failed) { /*std::cout << "FAILED TO EVAL EXPR!" << std::endl;*/ ERROR_RETURN }\
            TYPECHECK_STRICT(expr,a.second)\
            funcArgs.insert(std::make_pair(a.first,expr));}\
            }\
            {\


            #define BUILTIN_FUNC_END(name) }} // end name



            #define VOID_RETURN return nullptr;

        // std::cout << "Invoking Function:" << subject.data() << std::endl;

        /**
        @brief Prints a Message
         func print(message) -> void
        */

        BUILTIN_FUNC(BUILTIN_PRINT,{"msg",Object::Any})

            auto & arg = funcArgs["msg"];

            _print_to_stream(std::cout,arg);

            VOID_RETURN

        BUILTIN_FUNC_END(BUILTIN_PRINT)

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

            auto t = CompiledTarget::Executable(n,srcs_array);
            if(engine->outputTargetOpts.os == TargetOS::Windows){
                t->output_ext = "exe";
            }

            targets.push_back(t);
            return new Object{Object::Target,t};

        BUILTIN_FUNC_END(BUILTIN_EXECUTABLE)

        *code = INVOKE_NOTBUILTIN;
        return nullptr;
    };
}

