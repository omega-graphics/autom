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
    public:
        explicit GenNinja(OutputTargetOpts &outputOpts,GenNinjaOpts & _opts):opts(_opts),
        mainNinja(std::filesystem::path(_opts.outputDir.data()).append("build.ninja")),
        outputOpts(outputOpts){
//            if(!mainNinja.is_open())
//                // FAILED TO OUTPUT TO DIR!!
            mainNinja << "# " << GEN_FILE_HEADER << std::endl;
            mainNinja << "ninja_required_version = " << NINJA_MINIMUM_VERSION_REQUIRED << std::endl;
        };
        typedef enum : int {
            Flags,
            IncludeDirs,
            Libs
        } BuildRuleType;
        void writeBuildRuleParam(StrRef paramName,eval::Array *paramArgs,BuildRuleType type){
            mainNinja << INDENT << paramName << "=";
            if(!paramArgs->empty()){
                mainNinja << "\"\"";
            }
            else {
                toolchain->formatter.startCommandFormat(Toolchain::Formatter::unknown);
                switch (type) {
                    case Flags : {
                        toolchain->formatter.writeFlags(paramArgs->toStringVector());
                        break;
                    }
                    case IncludeDirs : {
                        toolchain->formatter.writeIncludes(paramArgs->toStringVector());
                        break;
                    }
                    case Libs : {
                        toolchain->formatter.writeLibs(paramArgs->toStringVector());
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
            return phony_name;
        };
        void consumeTarget(Target *target) override{

             if(target->type & COMPILED_OUTPUT_TARGET) {
                    auto *t =(CompiledTarget *)target;
                    // 1. Generate Source Build Recipes
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
                        writeBuildRuleParam("CFLAGS",t->cflags,Flags);


                        // Write INCLUDE_DIRS
                        writeBuildRuleParam("INCLUDE_DIRS",t->include_dirs,IncludeDirs);

                    }
                    // 2. Generate Link Recipe, if needed.
                    std::string phony_name;
                    switch (t->type) {
                        case EXECUTABLE : {
                            writeLinkRecipe(t,"exe");
                            break;
                        }
                        case SHARED_LIBRARY : {
                            writeLinkRecipe(t,"so");
                            break;
                        }
                        case STATIC_LIBRARY : {
                            writeLinkRecipe(t,"ar");
                            break;
                        }
                    }

                    if(t->type & (EXECUTABLE | SHARED_LIBRARY)){
                        writeBuildRuleParam("LDFLAGS",t->ldflags,Flags);
                    }

                    if(t->type != SOURCE_GROUP){
                        /// Any Compiled Target Except for Source Group
                        writeBuildRuleParam("LIBS",t->libs,Libs);
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
        inline void writeToolchainRule(StrRef ruleName,Toolchain::Formatter::CommandFormatType format,StrRef inputTemplate,std::ostream & toolchain_file){
            toolchain_file << std::endl << "rule " << ruleName << std::endl << "  command = ";

            toolchain->formatter.startCommandFormat(format);
            toolchain->formatter.writeCommandPrefix();
            toolchain->formatter.writeString(inputTemplate);
            toolchain->formatter.writeOutput(" $out ");
            toolchain->formatter.writeSource(" $in ");
            toolchain->formatter.endCommandFormat(toolchain_file);
        }
        void genToolchainRules(std::shared_ptr<Toolchain> & _toolchain) override {

            toolchain = _toolchain;

        #define CFAMILY_C_INPUT_TEMPLATE " $CFLAGS $INCLUDE_DIRS "
        #define CFAMILY_LD_INPUT_TEMPLATE " $LDFLAGS $LIBS "
        #define CFAMILY_AR_INPUT_TEMPLATE " $ARFLAGS $LIBS "

            std::ofstream toolchain_file(std::filesystem::path(opts.outputDir.data()).append("toolchain.ninja"));
            toolchain_file << "#" << GEN_FILE_HEADER << std::endl;
            if(toolchain->toolchainType & TOOLCHAIN_CFAMILY_ASM){
                writeToolchainRule("cc",Toolchain::Formatter::cc,CFAMILY_C_INPUT_TEMPLATE,toolchain_file);
                writeToolchainRule("cxx",Toolchain::Formatter::cxx,CFAMILY_C_INPUT_TEMPLATE,toolchain_file);
                writeToolchainRule("exe",Toolchain::Formatter::exe,CFAMILY_LD_INPUT_TEMPLATE,toolchain_file);
                writeToolchainRule("so",Toolchain::Formatter::so,CFAMILY_LD_INPUT_TEMPLATE,toolchain_file);
                writeToolchainRule("ar",Toolchain::Formatter::ar,CFAMILY_AR_INPUT_TEMPLATE,toolchain_file);
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
