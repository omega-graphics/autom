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
            mainNinja << "ninja_required_version =" << NINJA_MINIMUM_VERSION_REQUIRED << std::endl;
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
                        mainNinja << INDENT"CFLAGS =";
                        if(!t->cflags.empty()){
                            mainNinja << "\"\"";
                        }
                        else {
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::unknown);
                            toolchain->formatter.writeFlags(t->cflags);
                            toolchain->formatter.endCommandFormat(mainNinja);
                        }
                        mainNinja << std::endl;

                        // Write INCLUDE_DIRS
                        mainNinja << INDENT"INCLUDE_DIRS =";
                        if(t->include_dirs.empty()){
                            mainNinja << "";
                        }
                        else {
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::unknown);
                            toolchain->formatter.writeIncludes(t->include_dirs);
                            toolchain->formatter.endCommandFormat(mainNinja);
                        }
                        mainNinja << std::endl << std::endl;

                    }
                    // 2. Generate Link Recipe, if needed.
                    std::string phony_name;
                    switch (t->type) {
                        case EXECUTABLE : {
                            mainNinja << "build " << t->name;
                            if(!t->output_ext.empty())
                                mainNinja << "." << t->output_ext;

                            if(!t->output_ext.empty())
                                phony_name = t->name + "." + t->output_ext;
                            else
                                phony_name = t->name;
                            mainNinja << ": exe ";
                            for(auto & obj_src_p : t->source_object_map){
                                mainNinja << obj_src_p.second << " ";
                            }
                            break;
                        }
                        case SHARED_LIBRARY : {
                            mainNinja << "build " << t->name;
                            if(!t->output_ext.empty())
                                mainNinja << "." << t->output_ext;

                            if(!t->output_ext.empty())
                                phony_name = t->name + "." + t->output_ext;
                            else
                                phony_name = t->name;
                            mainNinja << ": so ";
                            for(auto & obj_src_p : t->source_object_map){
                                mainNinja << obj_src_p.second << " ";
                            }
                        }
                        case STATIC_LIBRARY : {
                            mainNinja << "build " << t->name;
                            if(!t->output_ext.empty())
                                mainNinja << "." << t->output_ext;

                            if(!t->output_ext.empty())
                                phony_name = t->name + "." + t->output_ext;
                            else
                                phony_name = t->name;
                            mainNinja << ": ar ";
                            for(auto & obj_src_p : t->source_object_map){
                                mainNinja << obj_src_p.second << " ";
                            }
                        }
                    }

                    if(t->type & EXECUTABLE | SHARED_LIBRARY){
                        mainNinja << std::endl;
                        mainNinja << INDENT"LDFLAGS =";
                        if(t->ldflags.empty()){
                            mainNinja << "";
                        }
                        else {
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::unknown);
                            toolchain->formatter.writeFlags(t->ldflags);
                            toolchain->formatter.endCommandFormat(mainNinja);
                        }
                    }

                    if(t->type != SOURCE_GROUP){
                        /// Any Compiled Target Except for Source Group
                        mainNinja << std::endl;
                        mainNinja << INDENT"LIBS =";
                        if(t->libs.empty()){
                            mainNinja << "";
                        }
                        else {
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::unknown);
                            toolchain->formatter.writeLibs(t->libs);
                            toolchain->formatter.endCommandFormat(mainNinja);
                        }
                    }
                    mainNinja << std::endl;

                    mainNinja << "build " << t->name << ": phony " << phony_name << std::endl;
            }
             else if(target->type == FS_ACTION) {

             }
        };
        bool supportsCustomToolchainRules() override {
            return true;
        };
        void genToolchainRules(std::shared_ptr<Toolchain> & _toolchain) override {

            toolchain = _toolchain;

        #define CFAMILY_C_INPUT_TEMPLATE " $CFLAGS $INCLUDE_DIRS "
        #define CFAMILY_LD_INPUT_TEMPLATE " $LDFLAGS $LIBS "
        #define CFAMILY_AR_INPUT_TEMPLATE " $ARFLAGS $LIBS "

            std::ofstream toolchain_file(std::filesystem::path(opts.outputDir.data()).append("toolchain.ninja"));
            toolchain_file << "#" << GEN_FILE_HEADER << std::endl;
            if(toolchain->toolchainType & TOOLCHAIN_CFAMILY_ASM){
                toolchain_file << std::endl << "rule cc" << std::endl << " command = ";

                toolchain->formatter.startCommandFormat(Toolchain::Formatter::cc);
                toolchain->formatter.writeCommandPrefix();
                toolchain->formatter.writeString(CFAMILY_C_INPUT_TEMPLATE);
                toolchain->formatter.writeOutput(" $out ");
                toolchain->formatter.writeSource(" $in ");
                toolchain->formatter.endCommandFormat(toolchain_file);

                toolchain_file << std::endl << "rule cxx" << std::endl << " command = ";

                toolchain->formatter.startCommandFormat(Toolchain::Formatter::cxx);
                toolchain->formatter.writeCommandPrefix();
                toolchain->formatter.writeString(CFAMILY_C_INPUT_TEMPLATE);
                toolchain->formatter.writeOutput(" $out ");
                toolchain->formatter.writeSource(" $in ");
                toolchain->formatter.endCommandFormat(toolchain_file);

                toolchain_file << std::endl << "rule exe" << std::endl << " command = ";

                toolchain->formatter.startCommandFormat(Toolchain::Formatter::exe);
                toolchain->formatter.writeCommandPrefix();
                toolchain->formatter.writeString(CFAMILY_LD_INPUT_TEMPLATE);
                toolchain->formatter.writeOutput(" $out ");
                toolchain->formatter.writeSource(" $in ");
                toolchain->formatter.endCommandFormat(toolchain_file);

                toolchain_file << std::endl << "rule so" << std::endl << " command = ";

                toolchain->formatter.startCommandFormat(Toolchain::Formatter::so);
                toolchain->formatter.writeCommandPrefix();
                toolchain->formatter.writeString(CFAMILY_LD_INPUT_TEMPLATE);
                toolchain->formatter.writeOutput(" $out ");
                toolchain->formatter.writeSource(" $in ");
                toolchain->formatter.endCommandFormat(toolchain_file);

                toolchain_file << std::endl << "rule ar" << std::endl << " command = ";

                toolchain->formatter.startCommandFormat(Toolchain::Formatter::ar);
                toolchain->formatter.writeCommandPrefix();
                toolchain->formatter.writeString(CFAMILY_AR_INPUT_TEMPLATE);
                toolchain->formatter.writeOutput(" $out ");
                toolchain->formatter.writeSource(" $in ");
                toolchain->formatter.endCommandFormat(toolchain_file);
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