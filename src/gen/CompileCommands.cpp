#include "../Target.h"
#include "../Gen.h"
#include "Toolchain.h"

#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <iostream>
#include <fstream>


namespace autom {
    class GenCompileCommands : public Gen {
        Toolchain *toolchain;

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

        void genToolchainRules() override {

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
                    for(autom::StrRef s : t->sources){

                    };
                    break;
                }
            }
        };

        void finish() override {
            w.EndArray();
        };

        GenCompileCommands():out("./hello.poop"),wrap_out(out),w(wrap_out){
           w.StartArray();
        };

        ~GenCompileCommands(){

        };
    };
};