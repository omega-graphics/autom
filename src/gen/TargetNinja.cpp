#include "../Gen.h"

namespace autom {

    class GenNinja : public Gen {

        void consumeTarget(Target *target) override{
            
        };
        bool supportsCustomToolchainRules() override {
            return true;
        };
        void genToolchainRules() override {

        };
        void finish() override {

        };
        
    };

    Gen *TargetNinja(){
        return new GenNinja();
    };
};