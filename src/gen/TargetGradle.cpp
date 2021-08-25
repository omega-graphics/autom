#include "Gen.h"

#include <fstream>
#include <filesystem>

namespace autom {


class GradleGen : public Gen {
    std::ofstream out;
public:
    
    GradleGen(autom::StrRef targetName,autom::StrRef outputDir){
        out.open(std::filesystem::path(outputDir).append("build.gradle"));
    };
    
    
    bool supportsCustomToolchainRules() override {
        return false;
    }
    
    void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) override {
        
    }
    
    void consumeTarget(Target *target) override{
        
    }
    
    void finish() override {
        out.close();
    }
    
    
};

}
