

#include "Execution.h"
#include "Builtins.def"
#include "ExecEngine.h"
#include "Diagnostic.h"
#include <initializer_list>
#include <iostream>
#include <fstream>
#include <utility>

#ifdef _WIN32
#include <Windows.h>
#endif

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
                case eval::Object::Namespace : {
                    out << "Namespace";
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
                out << "\x1b[33m" << std::quoted(castToString(obj)->value().data()) << "\x1b[0m";
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
                break;
            }
            case Object::Boolean : {
                auto *boolean = castToBool(obj);
                out << "\x1b[35m" << std::boolalpha << boolean->value() << std::noboolalpha << "\x1b[0m";
                break;
            }
            case Object::Target : {
                auto *target_wrapper = (TargetWrapper *)obj;
                auto *t = target_wrapper->value();
                out << "\x1b[38mTarget\x1b[0m {" << std::endl;
                out << "    name:";
                _print_to_stream(out,t->name);
                out << std::endl << "   type:" << std::hex << t->type << std::dec;
                out << std::endl << "}" << std::endl;
                break;
            }
            case Object::Namespace : {
                auto *ns = (Namespace *)obj;
                out << "{";
                auto ns_ref = ns->value();
                for(auto ent_it = ns_ref.begin();ent_it != ns_ref.end();ent_it++){
                    if(ent_it != ns_ref.begin()){
                        out << "\n,";
                    }
                    auto & e = *ent_it;
                    out << "    \x1b[39m" << e.first << "\x1b[0m" << ":";
                    _print_to_stream(out,e.second);
                }
                out << "}";
                break;
            }
        }
    };

    struct EvalContext {
        Eval *eval;
        ExecEngine *execEngine;
        ASTScope *currentScope;
        int code;
        inline void logError(const std::string & msg){
            execEngine->printError(msg);
        };
        inline void setCode(int c){
            code = c;
        };
    };


    void resolveSources(eval::Array *srcs,std::filesystem::path & currentDir){
        for(auto it = srcs->getBeginIterator();it != srcs->getEndIterator();it++){
            eval::String * obj = (eval::String *)*it;
            assert(obj != nullptr && objectIsString(obj) && "Object in sources is not a String.");
            obj->assign(std::filesystem::path(currentDir).append(obj->value().data()).lexically_normal().string());
        }
    };


    Object *bf_print(MapRef<std::string,Object *> args,EvalContext & ctxt){
        _print_to_stream(std::cout,args["msg"]);
        std::cout << std::endl;
        return nullptr;
    };



    Object *bf_project(MapRef<std::string,Object *> args,EvalContext & ctxt){
        
        auto *name = castToString(args["name"]);
        auto *version = castToString(args["version"]);

        std::cout << "Configuring Project " << name->value().data() << " " << version->value().data() << std::endl;
        
        GenContext context {{name->value(),version->value()},{},ctxt.execEngine->opts.outputDir};
        
        auto it = ctxt.eval->projects.insert(std::make_pair(std::move(context),std::deque<std::shared_ptr<Target>>{}));
        ctxt.eval->currentGenContext = (GenContext *)&it.first->first;

        return nullptr;
    };


    Object *bf_install_targets(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *targets = castToArray(args["targets"]);
        auto *dest = castToString(args["dest"]);
        auto rule = std::make_shared<TargetInstallRule>();
        rule->prefixed_dest = dest->value();
        
        auto & current_project_targets = ctxt.eval->projects[*ctxt.eval->currentGenContext];
        
        auto & res_targets = rule->targets;
        auto _ts = targets->toStringVector();
        for(auto & tname : _ts){
            auto pred = [&](std::shared_ptr<Target> & target){
                return target->name->value() == tname;
            };
            auto t = std::find_if(current_project_targets.begin(),current_project_targets.end(),pred);
            if(t == current_project_targets.end()){
                ctxt.execEngine->printError(formatmsg("@0 is not a target defined in the current project @1",tname,ctxt.eval->currentGenContext->projectDesc.name));
                return nullptr;
            }
            else {
                res_targets.push_back(*t);
            }
        }
        ctxt.eval->currentGenContext->installRules.push_back(rule);
        
        return nullptr;
        
    }

    Object *bf_install_files(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *files = castToArray(args["files"]);
        auto *dest = castToString(args["dest"]);

        auto rule = std::make_shared<FileInstallRule>();
        
        rule->prefixed_dest = dest->value();
        rule->files = files->toStringVector();
        
        ctxt.eval->currentGenContext->installRules.push_back(rule);
        return nullptr;
    }

    Object *bf_Executable(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);
        
        resolveSources(srcs,ctxt.eval->currentEvalDir);

        auto t = CompiledTarget::Executable(name,srcs);
        if(ctxt.execEngine->outputTargetOpts.os == TargetOS::Windows){
            t->output_ext->assign("exe");
        }

        ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };

    Object *bf_Archive(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);
        
        resolveSources(srcs,ctxt.eval->currentEvalDir);
        
        auto t = CompiledTarget::Archive(name,srcs);
        
        if(ctxt.execEngine->outputTargetOpts.os == TargetOS::Windows){
            t->output_ext->assign("lib");
        }
        else {
            t->output_ext->assign("a");
        }

        ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };


    Object *bf_Shared(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);
        auto t = CompiledTarget::Shared(name,srcs);
        
        resolveSources(srcs,ctxt.eval->currentEvalDir);
        
        if(ctxt.execEngine->outputTargetOpts.os == TargetOS::Windows){
            t->implib_ext = new eval::String("lib");
            t->output_ext->assign("dll");
        }
        else if(ctxt.execEngine->outputTargetOpts.os == TargetOS::Darwin){
            t->output_ext->assign("dylib");
        }
        else {
            t->output_ext->assign("so");
        }

        ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };

    Object *bf_SourceGroup(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToArray(args["sources"]);
        auto t = CompiledTarget::SourceGroup(name,srcs);
        
        resolveSources(srcs,ctxt.eval->currentEvalDir);
        
        ctxt.eval->addTarget(t);

        return new TargetWrapper(t);
    };

    Object *bf_GroupTarget(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *deps = castToArray(args["deps"]);
        auto t = GroupTarget::Create(name,deps);
        
        ctxt.eval->addTarget(t);
        
        return new TargetWrapper(t);
    }

    Object *bf_Script(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *cmd = castToString(args["cmd"]);
        auto *_args = castToArray(args["args"]);
        auto *outputs = castToArray(args["outputs"]);
        if(outputs->empty()){
            ctxt.execEngine->printError("Script targets must have at least 1 output file.");
            return nullptr;
        }
        
        auto t = ScriptTarget::Create(name,cmd,_args,outputs);
        
        ctxt.eval->addTarget(t);
        
        return new TargetWrapper(t);
    }

    

    Object *bf_JarLib(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToString(args["source_dir"]);
        
        srcs->assign(std::filesystem::path(ctxt.eval->currentEvalDir).append(srcs->value().data()).lexically_normal().string());
        
        auto t = JavaTarget::JarLib(name,srcs);
        
        return new TargetWrapper(t);
    }

    Object *bf_JarExe(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *name = castToString(args["name"]);
        auto *srcs = castToString(args["source_dir"]);
        
        srcs->assign(std::filesystem::path(ctxt.eval->currentEvalDir).append(srcs->value().data()).lexically_normal().string());
        
        auto t = JavaTarget::JarExe(name,srcs);
        
        return new TargetWrapper(t);
    }



    Object *bf_find_program(MapRef<std::string,Object *> args,EvalContext & ctxt){
#ifdef _WIN32
        std::string path;
        
        path.resize(2 ^(sizeof(size_t) - 1));
        size_t newSize = (size_t)GetEnvironmentVariableA("Path",path.data(),path.size());
        path.resize(newSize);
#else
        std::string path = std::getenv("PATH");
#endif
        std::string progPath;
        auto *progCmd = castToString(args["cmd"]);
        auto progFound = autom::locateProgram(progCmd->value(),path,progPath);
        if(progFound)
            return new eval::String(progPath);
        else
            return nullptr;
    }


    class SourceFileConfigDriver {
        std::istream & in;
        std::ostream & out;
    public:
        explicit SourceFileConfigDriver(std::istream &in,std::ostream &out) :out(out),in(in) {

        };
        void performConfiguration(EvalContext & ctxt){

            auto getChar = [&](){
                return char(in.get());
            };

            auto aheadChar = [&](){
                auto c = (char)in.get();
                in.seekg(-1,std::ios::cur);
                return c;
            };

            auto isIdentifierChar = [=](char & c){
                return (bool)std::isalnum(c) || c == '_';
            };

            char c;
            while((c = getChar()) != -1){
                switch (c) {
                    case '@': {
                        char at = c;
                        std::string variable;

                        while((c = getChar()) != '@'){
                            if(!isIdentifierChar(c)){
                                out << formatmsg("@0@1",at,variable).res;
                                continue;
                            }
                            variable.push_back(c);
                        }

                        auto object = ctxt.eval->referVarWithScope(ctxt.currentScope,variable);
                        if(object != nullptr) {
                            if(objectIsString(object)){
                                out << castToString(object)->value();
                            }
                            else {
                                ctxt.logError("Unsupported variable type");
                                return;
                            }
                        }


                        break;
                    }
                    default: {
                        out << c;
                        break;
                    }
                }
            }
        }
    };

    Object *bf_config_file(MapRef<std::string,Object *> args,EvalContext & ctxt){

        auto *inFile = castToString(args["input"]);
        auto *outFile = castToString(args["output"]);

        if(!std::filesystem::exists(inFile->value().data())){
            ctxt.logError("Input file in config_file function does not exist");
        }
        auto inFileDir = std::filesystem::path(outFile->value().data()).parent_path();
        if(!std::filesystem::exists(inFileDir)){
            std::filesystem::create_directories(inFileDir);
        }

        std::ifstream in(inFile->value().data(),std::ios::in);
        std::ofstream out(outFile->value().data(),std::ios::out);

        SourceFileConfigDriver configDriver {in,out};
        configDriver.performConfiguration(ctxt);


        return nullptr;
    }

    Object *bf_subdir(MapRef<std::string,Object *> args,EvalContext & ctxt){
        auto *dir = castToString(args["path"]);
        if(!std::filesystem::is_directory(dir->value().data())){
            ctxt.execEngine->printError(formatmsg("@0 is not a directory",dir->value()));
            return nullptr;
        };
        auto path = std::filesystem::path(dir->value().data()).append("AUTOM.build");
        ctxt.eval->importFile(path.string());
        return nullptr;
    }


    Object * Eval::tryInvokeBuiltinFunc(autom::StrRef subject,std::unordered_map<std::string,ASTExpr *> & args,int * code){

        *code = INVOKE_SUCCESS;

        std::vector<std::pair<std::string,Object *>> ready_args;

        auto checkArgs = [&](std::initializer_list<std::pair<CString,Object::Ty>> params){
            #define TYPECHECK_STRICT(object,t) if((t) != Object::Any && (object)->type != (t)){ engine->printError(formatmsg("Param `@0` is not a typeof @1. Instead got type @2",autom::StrRef(p.first),t,(object)->type));return false; }
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

        BUILTIN_FUNC(BUILTIN_PROJECT,bf_project,{"name",Object::String},{"version",Object::String});
        
        BUILTIN_FUNC(BUILTIN_INSTALL_FILES,bf_install_files,{"files",Object::Array},{"dest",Object::String});
        
        BUILTIN_FUNC(BUILTIN_INSTALL_TARGETS,bf_install_targets,{"targets",Object::Array},{"dest",Object::String});
         
        BUILTIN_FUNC(BUILTIN_EXECUTABLE,bf_Executable,{"name",Object::String},{"sources",Object::Array});

        BUILTIN_FUNC(BUILTIN_ARCHIVE,bf_Archive,{"name",Object::String},{"sources",Object::Array});

        BUILTIN_FUNC(BUILTIN_SHARED,bf_Shared,{"name",Object::String},{"sources",Object::Array});
        
        BUILTIN_FUNC(BUILTIN_SOURCE_GROUP,bf_SourceGroup,{"name",Object::String},{"sources",Object::Array});
        
        BUILTIN_FUNC(BUILTIN_GROUP_TARGET,bf_GroupTarget,{"name",Object::String},{"deps",Object::Array});
        
        BUILTIN_FUNC(BUILTIN_SCRIPT,bf_Script,
                     {"name",Object::String},
                     {"cmd",Object::String},
                     {"args",Object::Array},
                     {"outputs",Object::Array});

        BUILTIN_FUNC(BUILTIN_FIND_PROGRAM,bf_find_program,{"cmd",Object::String});

        BUILTIN_FUNC(BUILTIN_CONFIG_FILE,bf_config_file,{"in",Object::String},{"out",Object::String});
        
        BUILTIN_FUNC(BUILTIN_SUBDIR,bf_subdir,{"path",Object::String});
        
        *code = INVOKE_NOTBUILTIN;
        return nullptr;
    };
}

