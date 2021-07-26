#include "../Target.h"
#include "../Gen.h"
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <iostream>
#include <fstream>

namespace autom {
    class GenCompileCommands : public Gen {
        std::ofstream out;
        rapidjson::OStreamWrapper wrap_out;
        rapidjson::Writer<rapidjson::OStreamWrapper> w;
        void consumeTarget(Target *target) override {
            w.StartObject();
            w.Key("Hello");
            w.StartObject();
            w.Key("Else");
            w.String("hello");
            w.EndObject();
            w.EndObject();
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