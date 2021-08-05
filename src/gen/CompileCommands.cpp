#include "../Target.h"
#include "../Gen.h"
#include "Toolchain.h"

#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <iostream>
#include <fstream>

#include <filesystem>


namespace autom {
    class GenCompileCommands final : public Gen {
        std::shared_ptr<Toolchain> toolchain;

        std::ofstream out;
        rapidjson::OStreamWrapper wrap_out;
        rapidjson::Writer<rapidjson::OStreamWrapper> w;
    private:
        inline void writeCommand(const std::string& cmd,const autom::StrRef & dir,const autom::StrRef & file){
            w.StartObject();
            w.Key("command");
            w.String(cmd.c_str(),cmd.size());
            w.Key("directory");
            w.String(dir.data(),dir.size());
            w.Key("file");
            w.String(file.data(),file.size());
            w.EndObject();
        }
    public:

        bool supportsCustomToolchainRules() override {
            return true;
        }

        void genToolchainRules(std::shared_ptr<Toolchain> & _toolchain) override {
            toolchain = _toolchain;
        }

        void consumeTarget(Target *target) override {
            switch (target->type) {

                case FS_ACTION : {
                    break;
                }
                case SCRIPT_ACTION : {
                    break;
                }
                default: {
                    auto * t = (CompiledTarget *)target;
                    for(auto & s_obj_pair : t->source_object_map){
                        auto & s = s_obj_pair.first;
                        std::ostringstream cmdOut;
                        if(isCSrc(s))
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::cc);
                        else if(isCXXSrc(s))
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::cxx);
                        else if(isOBJCSrc(s))
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::objc);
                        else if(isOBJCXXSrc(s))
                            toolchain->formatter.startCommandFormat(Toolchain::Formatter::objcxx);

                        toolchain->formatter.writeCommandPrefix();
                        toolchain->formatter.writeDefines(t->defines->toStringVector());
                        toolchain->formatter.writeFlags(t->cflags->toStringVector());
                        toolchain->formatter.writeIncludes(t->include_dirs->toStringVector());
                        toolchain->formatter.writeSource(s);
                        toolchain->formatter.writeOutput(s_obj_pair.second);
                        toolchain->formatter.endCommandFormat(cmdOut);
                        auto dir = std::filesystem::path(s.data()).root_directory();
                        writeCommand(cmdOut.str(),dir.string().data(),s);
                    };
                    break;
                }
            }
        };

        void finish() override {
            w.EndArray();
        };

        explicit GenCompileCommands(autom::StrRef output_path):out(output_path),wrap_out(out),w(wrap_out){
           w.StartArray();
        };

        ~GenCompileCommands(){

        };
    };

    Gen *TargetCompileCommands(autom::StrRef output_path){
        return new GenCompileCommands(output_path);
    };
};