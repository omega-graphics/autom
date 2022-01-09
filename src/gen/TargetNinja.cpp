#include "../Gen.h"
#include "Autom.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Targets.def"
#include "Toolchains.def"

namespace autom {

#define NINJA_MINIMUM_VERSION_REQUIRED "1.8"
#define INDENT "  "

    class GenNinja final : public Gen {
        std::shared_ptr<Toolchain> toolchain;

        std::ofstream mainNinja;

        GenNinjaOpts & opts;

        OutputTargetOpts &outputOpts;
        
        ToolchainDefaults * toolchainDefaults;
        
        std::deque<std::string> scriptRules;
        
    public:
        explicit GenNinja(OutputTargetOpts &outputOpts,GenNinjaOpts & _opts):opts(_opts),
        outputOpts(outputOpts),toolchainDefaults(nullptr){
//            if(!mainNinja.is_open())
//                // FAILED TO OUTPUT TO DIR!!
            
        };
        typedef enum : int {
            Flags,
            IncludeDirs,
            Libs,
            LibDirs
#ifdef __APPLE__
            ,Frameworks,
            FrameworkDirs
#endif
        } BuildRuleType;
        
        void configGenContext() override {
            mainNinja.open(std::filesystem::path(context->outputDir.data()).append("build.ninja"));
            mainNinja << "# " << GEN_FILE_HEADER << std::endl;
            mainNinja << "ninja_required_version = " << NINJA_MINIMUM_VERSION_REQUIRED << std::endl;
        }
        
        void consumeToolchainDefaults(ToolchainDefaults &conf) override {
            toolchainDefaults = &conf;
        }
        
        void writeBuildRuleParam(StrRef paramName,const ArrayRef<std::string> & paramArgs,BuildRuleType type){
            mainNinja << INDENT << paramName << "=";
            if(paramArgs.empty()){
                mainNinja << "";
            }
            else {
                toolchain->formatter.startCommandFormat(Toolchain::Formatter::unknown);
                switch (type) {
                    case Flags : {
                        toolchain->formatter.writeFlags(paramArgs);
                        break;
                    }
                    case IncludeDirs : {
                        toolchain->formatter.writeIncludes(paramArgs);
                        break;
                    }
                    case Libs : {
                        toolchain->formatter.writeLibs(paramArgs);
                        break;
                    }
                    case LibDirs : {
                        toolchain->formatter.writeLibDirs(paramArgs);
                        break;
                    }
                    #ifdef __APPLE__
                    case Frameworks : {
                        toolchain->formatter.writeFrameworks(paramArgs);
                        break;
                    }
                    case FrameworkDirs : {
                        toolchain->formatter.writeFrameworkDirs(paramArgs);
                        break;
                    }
                    #endif
                }

                toolchain->formatter.endCommandFormat(mainNinja);
            }
            mainNinja << std::endl;
        }
        std::string writeLinkRecipe(std::shared_ptr<CompiledTarget> & t,StrRef toolName) {
            std::string phony_name = "";
            mainNinja << "build ";
            
            if(!t->output_dir->empty()){
                phony_name.append(t->output_dir->value()).append("/");
            }
            
            phony_name.append(t->name->value());
            
            if(!t->output_ext->empty()) {
                phony_name.append(".").append(t->output_ext->value());
            }

            mainNinja << phony_name;
               
            mainNinja << ": " << toolName << " ";
            for(auto & obj_src_p : t->source_object_map){
                mainNinja << obj_src_p.second << " ";
            }
            
            /// Must have a seperate output string stream because
            /// Source Group Targets' compiled objects will be added to the list of object files to link.
            
            std::ostringstream deps_out;
            
            if(!t->resolvedDeps.empty()) {
                /// Write Order Only Dependencies
                deps_out << " || ";
                for(auto d : t->resolvedDeps){
                    if(d->type & COMPILED_OUTPUT_TARGET){
                        auto t = std::dynamic_pointer_cast<CompiledTarget>(d);
                        /// For every other Compiled Target
                        if(d->type != SOURCE_GROUP){
                            if(!t->output_dir->empty()){
                                deps_out << t->output_dir->value() << "/";
                            };
                    
                            deps_out << t->name->value() << "." << t->output_ext->value();
                           
                        }
                        /// Only For Source Group Targets
                        else {
                            /// Output Source Group object files to `mainNinja` output stream link input.
                            for(auto & src_obj_pair : t->source_object_map){
                                mainNinja << src_obj_pair.second << " ";
                            }
                        }
                        deps_out << " ";
                        
                    }
                }
            }
            
            mainNinja << deps_out.str();
            
            mainNinja << std::endl;
            return phony_name;
        };
        void consumeTarget(std::shared_ptr<Target> & target) override{

             if(target->type & COMPILED_OUTPUT_TARGET) {
                    auto t = std::dynamic_pointer_cast<CompiledTarget>(target);
                    // 1. Generate Source Build Recipes
                    mainNinja << "#" << t->name->value() << " Sources" << std::endl;
                    for(auto & s_obj_pair : t->source_object_map){
                        auto & s = s_obj_pair.first;
                        mainNinja << "build " << s_obj_pair.second << ":";
                        mainNinja << INDENT;
                        if(isCSrc(s))
                            mainNinja << "cc";
                        else if(isCXXSrc(s))
                            mainNinja << "cxx";
                        else if(isOBJCSrc(s))
                            mainNinja << "objc";
                        else if(isOBJCXXSrc(s))
                            mainNinja << "objcxx";

                        mainNinja << " " << s << std::endl;

                        // Write CFLAGS
                        writeBuildRuleParam("CFLAGS",t->cflags->toStringVector(),Flags);


                        // Write INCLUDE_DIRS
                        writeBuildRuleParam("INCLUDE_DIRS",t->include_dirs->toStringVector(),IncludeDirs);

                    }
                    mainNinja << std::endl << "#" << t->name->value() << " Link Task" << std::endl;
                    // 2. Generate Link Recipe, if needed.
                    std::string phony_name;
                    switch (t->type) {
                        case EXECUTABLE : {
                            phony_name = writeLinkRecipe(t,"exe");
                            break;
                        }
                        case SHARED_LIBRARY : {
                            phony_name = writeLinkRecipe(t,"so");
                            break;
                        }
                        case STATIC_LIBRARY : {
                            phony_name = writeLinkRecipe(t,"ar");
                            break;
                        }
                    }

                    if(t->type & (EXECUTABLE | SHARED_LIBRARY)){
                        writeBuildRuleParam("LDFLAGS",t->ldflags->toStringVector(),Flags);
                    }

                    if(t->type != SOURCE_GROUP){
                        /// Any Compiled Target Except for Source Group
                        auto libs = t->libs->toStringVector();
                        
#ifdef __APPLE__
                        auto frameworks = t->frameworks->toStringVector();
                        /// Strip .framework extension from frameworks
                        for(auto & f : frameworks){
                            auto dot_end = f.size() - 10;
                            auto ext = f.substr(dot_end,10);
                            if(ext == ".framework"){
                                f.assign(f.substr(0,dot_end));
                            }
                        }
                        
                       
#endif
                         auto lib_dirs = t->lib_dirs->toStringVector();
                        
                        for(auto d : t->resolvedDeps){
                            if(d->type & (SHARED_LIBRARY | STATIC_LIBRARY)){
                                auto _ct = std::dynamic_pointer_cast<CompiledTarget>(d);
                                std::ostringstream dep_name;
                                
                                if(!_ct->output_dir->empty()){
//                                    auto found = std::find_if(lib_dirs.begin(),lib_dirs.end(),[&](std::string & str){
//                                        return _ct->output_dir->value() == str;
//                                    });
//                                    /// If lib dir has not been added yet.
//                                    if(found == lib_dirs.end()){
//                                    lib_dirs.push_back(_ct->output_dir->value());
//                                    }
                                    dep_name << _ct->output_dir->value() << "/";
                                }
                                /// Use IMPORT LIB when linking DLL on Windows
                                if(outputOpts.platform == TargetPlatform::Windows){
                                    if(_ct->type == SHARED_LIBRARY){
                                        dep_name << _ct->name->value() << "." << _ct->implib_ext->value();
                                    }
                                    else {
                                        dep_name << d->name->value() << "." << _ct->output_ext->value();
                                    }
                                }
                                else {
                                    if(toolchain->stripLibPrefix){
                                        std::string str = d->name->value();
    //                                    if(str.substr(0,3) == "lib"){
    //                                        dep_name << str.substr(3,str.size()-3);
    //                                    }
    //                                    else {
                                        dep_name << str << "." << _ct->output_ext->value();
//          
                                    }
                                    else {
                                        dep_name << d->name->value() << "." << _ct->output_ext->value();
                                    }
                                }
                               
                                
                                libs.push_back(dep_name.str());
                            }
                        }
                        writeBuildRuleParam("LIBS",libs,Libs);
                        
                    
                        
                        lib_dirs.push_back(".");
                        
                        writeBuildRuleParam("LIB_DIRS",lib_dirs,LibDirs);
                        
#ifdef __APPLE__
                        writeBuildRuleParam("FRAMEWORKS",frameworks,Frameworks);
                        
                        auto framework_dirs = t->framework_dirs->toStringVector();
                        
                        writeBuildRuleParam("FRAMEWORK_DIRS",framework_dirs,FrameworkDirs);
                        
#endif
                    }
                    mainNinja << std::endl;

                    if(!t->output_ext->empty())
                        mainNinja << "build " << t->name->value() << ": phony " << phony_name << std::endl;
            }
             else if(target->type == GROUP_TARGET){
                 auto t = std::dynamic_pointer_cast<GroupTarget>(target);
                 mainNinja << "build " << t->name->value() << ": phony ";
                 for(auto & d : t->resolvedDeps){
                     mainNinja << d->name->value() << " ";
                 }
                 mainNinja << std::endl;
             }
             else if(target->type & FS_ACTION) {
                 auto t = std::dynamic_pointer_cast<FSTarget>(target);
                 mainNinja << "build ";
                 auto _sources = t->sources->toStringVector();
                 if(t->type == FS_COPY){
                     auto dest = std::filesystem::path(t->dest->value().data());
                     std::ostringstream srcs_out("");
                     for(auto & s : _sources){
                         srcs_out << s;
                         mainNinja << dest.string() << "/" << std::filesystem::path(s).filename().string() << " ";
                     }
                     mainNinja << ": ";
                     mainNinja << "copy ";
                     mainNinja << srcs_out.str() << std::endl;
                 }
                 else if(t->type == FS_MKDIR){
                     auto dest = std::filesystem::path(t->dest->value().data());
                     mainNinja << dest.string() << ": mkdir " << dest.string() << std::endl;
                 }
                 else if(t->type == FS_SYMLINK){
                     auto dest = std::filesystem::path(t->dest->value().data());
                     
                     mainNinja << dest.string() << "/" << std::filesystem::path(t->symlink_src->value().data()).filename().string() << " ";
                     
                     mainNinja << ": ";
                     mainNinja << "smylink ";
                     mainNinja << t->symlink_src->value() << std::endl;
                 }
             }
             else if(target->type == SCRIPT_TARGET){
                 auto t = std::dynamic_pointer_cast<ScriptTarget>(target);
                 auto t_outputs = t->outputs->toStringVector();
                 mainNinja << "build ";
                 for(auto &t : t_outputs){
                     mainNinja << t << " ";
                 }
                 mainNinja << ":" << "script " << t->script->value() << std::endl;
                 mainNinja << " ARGS=";
                 if(!t->args->empty()){
                     auto _args = t->args->toStringVector();
                     for(auto & a : _args){
                         mainNinja << a << " ";
                     }
                 }
                 mainNinja << std::endl;
                 mainNinja << " NAME=" << t->name->value() << std::endl;
                 mainNinja << " DESC=" << t->desc->value() << std::endl;
                 
                 mainNinja << "build " << t->name->value() << ": phony ";
                 for(auto &t : t_outputs){
                     mainNinja << t << " ";
                 }
             }
        };
        bool supportsCustomToolchainRules() override {
            return true;
        };
        inline void writeToolchainRule(StrRef ruleName,Toolchain::Formatter::CommandFormatType format,StrRef inputTemplate,std::initializer_list<eval::Array *> defaults,std::ostream & toolchain_file,StrRef desc){
            toolchain_file << std::endl << "rule " << ruleName << std::endl << "  command = ";

            toolchain->formatter.startCommandFormat(format);
            toolchain->formatter.writeCommandPrefix();
            toolchain->formatter.writeString(INDENT);
            for(auto & d : defaults){
                toolchain->formatter.writeFlags(d->toStringVector());
            }
            toolchain->formatter.writeString(inputTemplate);
            toolchain->formatter.writeOutput("$out ");
            toolchain->formatter.writeSource(" $in ");
            toolchain->formatter.endCommandFormat(toolchain_file);
            
            toolchain_file << std::endl << "  description = " << desc << std::endl;
        }
        void genToolchainRules(std::shared_ptr<Toolchain> & _toolchain) override {

            toolchain = _toolchain;

        #define CFAMILY_C_INPUT_TEMPLATE " $CFLAGS $INCLUDE_DIRS "
#ifdef __APPLE__
    #define CFAMILY_LD_INPUT_TEMPLATE " $LDFLAGS $LIBS $LIB_DIRS $FRAMEWORKS $FRAMEWORK_DIRS "
    #define CFAMILY_AR_INPUT_TEMPLATE " $ARFLAGS $LIBS $LIB_DIRS $FRAMEWORKS $FRAMEWORK_DIRS "
#else
        #define CFAMILY_LD_INPUT_TEMPLATE " $LDFLAGS $LIBS $LIB_DIRS "
        #define CFAMILY_AR_INPUT_TEMPLATE " $ARFLAGS $LIBS $LIB_DIRS "
#endif

            std::ofstream toolchain_file(std::filesystem::path(context->outputDir.data()).append("toolchain.ninja"));
            toolchain_file << "#" << GEN_FILE_HEADER << std::endl;
            if(toolchain->toolchainType & TOOLCHAIN_CFAMILY_ASM){
                writeToolchainRule("cc",Toolchain::Formatter::cc,CFAMILY_C_INPUT_TEMPLATE,{toolchainDefaults->c_flags},toolchain_file,"CC $in");
                writeToolchainRule("cxx",Toolchain::Formatter::cxx,CFAMILY_C_INPUT_TEMPLATE,{toolchainDefaults->c_flags,toolchainDefaults->cxx_flags},toolchain_file,"CXX $in");
#ifdef __APPLE__
                writeToolchainRule("objc",Toolchain::Formatter::objc,CFAMILY_C_INPUT_TEMPLATE,{toolchainDefaults->c_flags},toolchain_file,"OBJC $in");
                writeToolchainRule("objcxx",Toolchain::Formatter::objcxx,CFAMILY_C_INPUT_TEMPLATE,{toolchainDefaults->c_flags,toolchainDefaults->cxx_flags},toolchain_file,"OBJCXX $in");
#endif
                writeToolchainRule("exe",Toolchain::Formatter::exe,CFAMILY_LD_INPUT_TEMPLATE,{},toolchain_file,"LINK_EXE $out");
                writeToolchainRule("so",Toolchain::Formatter::so,CFAMILY_LD_INPUT_TEMPLATE,{},toolchain_file,"LINK_SO $out");
                writeToolchainRule("ar",Toolchain::Formatter::ar,CFAMILY_AR_INPUT_TEMPLATE,{},toolchain_file,"AR $out");
            }
            /// Write Stamp Rule.
            toolchain_file << "rule stamp" << std::endl;
            toolchain_file << " command =";
#ifdef _WIN32
            toolchain_file << "echo > $out";
#else
            toolchain_file << "touch $out";
#endif
            toolchain_file << " description =" << "STAMP $out" << std::endl;
            
            /// Write Copy Rule
            toolchain_file << "rule copy" << std::endl;
            toolchain_file << " command =";
#ifdef _WIN32
            toolchain_file << "copy > $out";
#else
            toolchain_file << "cp -R $in $out";
#endif
            toolchain_file << " description =" << "COPY $in $out" << std::endl;
            
            /// Write Symlink Rule
            toolchain_file << "rule symlink" << std::endl;
            toolchain_file << " command =";
#ifdef _WIN32
            toolchain_file << "mklink $out $in";
#else
            toolchain_file << "ln -s $in $out";
#endif
            toolchain_file << " description =" << "SYMLINK $in $out"<< std::endl;
            
            
            /// Write Script Run Rule
            toolchain_file << "rule script" << std::endl;
            toolchain_file << " command =";
#ifdef _WIN32
            toolchain_file << "py -3 $in $ARGS";
#else
            toolchain_file << "python3 $in $ARGS";
#endif
            toolchain_file << " description =" << "$DESC"<< std::endl;
            
            toolchain_file << std::endl;
            toolchain_file.close();
            mainNinja << "include toolchain.ninja" << std::endl;
        };
        void finish() override {
            mainNinja.close();
        };
        
    };

    Gen *TargetNinja(OutputTargetOpts &outputOpts,GenNinjaOpts & opts){
        return new GenNinja(outputOpts,opts);
    };
};
