#include "../Gen.h"
#include "Autom.h"

#include <filesystem>
#include <fstream>
#include <iostream>

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
    public:
        explicit GenNinja(OutputTargetOpts &outputOpts,GenNinjaOpts & _opts):opts(_opts),
        mainNinja(std::filesystem::path(_opts.outputDir.data()).append("build.ninja")),
        outputOpts(outputOpts),toolchainDefaults(nullptr){
//            if(!mainNinja.is_open())
//                // FAILED TO OUTPUT TO DIR!!
            mainNinja << "# " << GEN_FILE_HEADER << std::endl;
            mainNinja << "ninja_required_version = " << NINJA_MINIMUM_VERSION_REQUIRED << std::endl;
        };
        typedef enum : int {
            Flags,
            IncludeDirs,
            Libs,
            LibDirs
        } BuildRuleType;
        
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
                }

                toolchain->formatter.endCommandFormat(mainNinja);
            }
            mainNinja << std::endl;
        }
        std::string writeLinkRecipe(CompiledTarget *t,StrRef toolName) {
            std::string phony_name;
            mainNinja << "build " << t->name->value();
            if(!t->output_ext->empty())
                mainNinja << "." << t->output_ext->value();

            if(!t->output_ext->empty())
                phony_name = std::string(t->name->value()) + "." + t->output_ext->value().data();
            else
                phony_name = t->name->value();
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
                        auto *t = (CompiledTarget *)d;
                        /// For every other Compiled Target
                        if(d->type != SOURCE_GROUP){
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
        void consumeTarget(Target *target) override{

             if(target->type & COMPILED_OUTPUT_TARGET) {
                    auto *t =(CompiledTarget *)target;
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
                        
                        for(auto d : t->resolvedDeps){
                            if(d->type & (SHARED_LIBRARY | STATIC_LIBRARY)){
                                auto _ct = (CompiledTarget *)d;
                                std::ostringstream dep_name;
                                if(toolchain->stripLibPrefix){
                                    std::string str = d->name->value();
                                    if(str.substr(0,3) == "lib"){
                                        dep_name << str.substr(3,str.size()-3);
                                    }
                                    else {
                                        dep_name << str;
                                    }
                                }
                                else {
                                    dep_name << d->name->value();
                                }
                                
                                libs.push_back(dep_name.str());
                            }
                        }
                        writeBuildRuleParam("LIBS",libs,Libs);
                        
                        auto lib_dirs = t->lib_dirs->toStringVector();
                        
                        lib_dirs.push_back(".");
                        
                        writeBuildRuleParam("LIB_DIRS",lib_dirs,LibDirs);
                    }
                    mainNinja << std::endl;

                    if(!t->output_ext->empty())
                        mainNinja << "build " << t->name->value() << ": phony " << phony_name << std::endl;
            }
             else if(target->type == FS_ACTION) {

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
            toolchain->formatter.writeOutput(" $out ");
            toolchain->formatter.writeSource(" $in ");
            toolchain->formatter.endCommandFormat(toolchain_file);
            
            toolchain_file << std::endl << "  description = " << desc << std::endl;
        }
        void genToolchainRules(std::shared_ptr<Toolchain> & _toolchain) override {

            toolchain = _toolchain;

        #define CFAMILY_C_INPUT_TEMPLATE " $CFLAGS $INCLUDE_DIRS "
        #define CFAMILY_LD_INPUT_TEMPLATE " $LDFLAGS $LIBS $LIB_DIRS "
        #define CFAMILY_AR_INPUT_TEMPLATE " $ARFLAGS $LIBS $LIB_DIRS "

            std::ofstream toolchain_file(std::filesystem::path(opts.outputDir.data()).append("toolchain.ninja"));
            toolchain_file << "#" << GEN_FILE_HEADER << std::endl;
            if(toolchain->toolchainType & TOOLCHAIN_CFAMILY_ASM){
                writeToolchainRule("cc",Toolchain::Formatter::cc,CFAMILY_C_INPUT_TEMPLATE,{toolchainDefaults->c_flags},toolchain_file,"CC $in");
                writeToolchainRule("cxx",Toolchain::Formatter::cxx,CFAMILY_C_INPUT_TEMPLATE,{toolchainDefaults->c_flags,toolchainDefaults->cxx_flags},toolchain_file,"CXX $in");
                writeToolchainRule("exe",Toolchain::Formatter::exe,CFAMILY_LD_INPUT_TEMPLATE,{},toolchain_file,"LINK_EXE $out");
                writeToolchainRule("so",Toolchain::Formatter::so,CFAMILY_LD_INPUT_TEMPLATE,{},toolchain_file,"LINK_SO $out");
                writeToolchainRule("ar",Toolchain::Formatter::ar,CFAMILY_AR_INPUT_TEMPLATE,{},toolchain_file,"AR $out");
            }
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
