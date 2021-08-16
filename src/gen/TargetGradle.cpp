#include "Gen.h"



namespace autom {


class GradleGen : public Gen {
    
    
    bool supportsCustomToolchainRules() override {
        return false;
    }
    
    void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) override {
        
    }
    
    void consumeTarget(Target *target) override{
        
    }
    
    void finish() override {
        
    }
    
    
};

}
