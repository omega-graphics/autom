#include "Gen.h"

#include "Diagnostic.h"

#include <fstream>
#include <filesystem>

namespace autom {

#define GRADLE_MINIMUM_SUPPORTED "4.5"

class GradleGen : public Gen {
    std::ofstream out;
public:
    
    explicit GradleGen(){
       
    };
    
    void configGenContext() override {
        out.open(std::filesystem::path(context->outputDir.data()).append("settings.gradle"));
    }
    
    void consumeToolchainDefaults(ToolchainDefaults &conf) override {
        
    }
    
    
    bool supportsCustomToolchainRules() override {
        return false;
    }
    
    void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) override {
        return;
    }
    
    void consumeTarget(std::shared_ptr<Target> & target) override{
        
        /// Create a new Gradle Subproject
        auto java_target = std::dynamic_pointer_cast<JavaTarget>(target);
        
        
        std::ofstream gradleTargetOut(std::filesystem::path(context->outputDir.data()).append(java_target->name->value().data()).append("build.gradle"),std::ios::out);
        
        
        if(!java_target->maven_repos->empty()){
            gradleTargetOut << "repositories {" << std::endl;
            auto v = java_target->maven_repos->toStringVector();
            for(StrRef repo : v){
                gradleTargetOut << "    " << repo << "()" << std::endl;
            }
            gradleTargetOut << "}" << std::endl;
        }
        
        if(!java_target->maven_deps->empty()){
            gradleTargetOut << "dependencies {" << std::endl;
            auto v = java_target->maven_deps->toStringVector();
            for(StrRef dep : v){
                gradleTargetOut << "    implementation \"" << dep << "\"" << std::endl;
            }
            gradleTargetOut << "}" << std::endl;
        }
       
        
        if(java_target->type == JAR_LIB){
            /// Generate Jar Lib Rules for Gradle
            gradleTargetOut << R"(
            
            plugins {
                id 'java-library'
            }

            )";
            
        }
        else if(java_target->type == JAR_EXE){
            /// Generate Jar Exe Rules for Gradle
            gradleTargetOut << R"(
            
            plugins {
                id 'application'
            }

            )";
        }
        
        gradleTargetOut <<
formatmsg(R"(sourceSets {
     main {
        java {
            srcDir '@0'
        }
     }
})",java_target->src_dir->value()).res;
        gradleTargetOut << "}" << std::endl;
//        else {
//
//        }
        
        gradleTargetOut.close();
        
        out << "include '" << java_target->name->value().data() << "'" << std::endl;
    }
    
    void finish() override {
        out.close();
    }
    
    
};



Gen *TargetGradle(){
    return new GradleGen();
}

}
