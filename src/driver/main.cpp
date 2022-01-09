#include "ExecEngine.h"
#include "ADT.h"
#include "Gen.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>


inline void printHelp(){
    std::cout << 
    R"(AUTOM Build Tool
================
Usage: autom  [options] output-dir
--------------------------------------------------

Options:


--help, -h                --> Show this message
    
--interface-path, -I      --> Add a path to the interface file (*.autom) search path list.
    
--toolchains              --> Sets the toolchains config file.
                              (Declares all existing toolchains and what flags to use)

--platform                --> Sets the target platform (Uses host as default)
                              Choices:[windows|macos|linux|android|ios]

--os                      --> Sets the target os (Uses host as default)
                              Choices:[windows|darwin|linux]

--arch                    --> Sets the target cpu architecture (Uses host as default)
                              Choices:[x86|x86_64|arm|aarch64]
--define, -D              --> Defines a variable with an intial value.

    Modes:

    --ninja (default)     --> Generate Ninja Build Files
    --sln                 --> Generate a Visual Studio Solution
    --xcode               --> Generate an Xcode Project
    
    Mode Options:

    Ninja:

    --split-all          --> Create a subninja for every compiled target
    
    Visual Studio:

    --version <version>  --> Targets Solution for the provided version
    
    Xcode:

    --new-build          --> Use New Build System in Xcode
)" << std::endl;
    exit(0);
};

int main(int argc,char * argv[]){
    
    auto exec_path = std::filesystem::path(argv[0]).parent_path().parent_path().string();
    
    if(exec_path.empty()){
        autom::locateProgram(argv[0],std::getenv("PATH"),exec_path);
        exec_path = std::filesystem::path(exec_path).parent_path().parent_path().string();
    }
    
    std::cout << exec_path << std::endl;
    
    auto std_modules_path = std::filesystem::path(exec_path).append("modules").string();
    
    std::vector<autom::StrRef> interfaceSearchPaths = {std_modules_path};

    autom::StrRef outputDir = argv[argc-1];
    std::string toolchainFile;

    autom::TargetOS targetOS = autom::hostOS();
    autom::TargetArch targetArch = autom::hostArch();
    autom::TargetPlatform targetPlatform = autom::hostPlatform();
    
    std::vector<std::pair<std::string,autom::eval::Object *>> defines;

    bool ninja = true;
    bool sln = false;
    bool xcode = false;


    for(unsigned i = 1;i < argc;i++){
        autom::StrRef flag(argv[i]);
        if(flag == "--help" || flag == "-h"){
            printHelp();
        }


        else if(flag == "--platform"){
            autom::StrRef val {argv[++i]};
            if(val == "windows")
                targetPlatform = autom::TargetPlatform::Windows;
            else if(val == "macos")
                targetPlatform = autom::TargetPlatform::macOS;
            else if(val == "ios")
                targetPlatform = autom::TargetPlatform::iOS;
            else if(val == "android")
                targetPlatform = autom::TargetPlatform::Android;
            else if(val == "linux")
                targetPlatform = autom::TargetPlatform::Linux;
        }
        else if(flag == "--arch"){
            autom::StrRef val {argv[++i]};
            if(val == "x86")
                targetArch = autom::TargetArch::x86;
            else if(val == "x86_64")
                targetArch = autom::TargetArch::x86_64;
            else if(val == "arm")
                targetArch = autom::TargetArch::ARM;
            else if(val == "aarch64")
                targetArch = autom::TargetArch::AARCH64;
        }
        else if(flag == "--os"){
            autom::StrRef val {argv[++i]};
            if(val == "windows")
                targetOS = autom::TargetOS::Windows;
            else if(val == "darwin")
                targetOS = autom::TargetOS::Darwin;
            else if(val == "linux")
                targetOS = autom::TargetOS::Linux;
        }
        
        else if(flag == "--toolchains"){
            autom::StrRef val {argv[++i]};
            toolchainFile = val;
        }
        
        else if(flag == "--interface-path" || flag == "-I"){
            autom::StrRef val {argv[++i]};
            interfaceSearchPaths.push_back(val);
        }

        else if(flag == "--ninja"){
            ninja = true;
        }
        else if(flag == "--sln"){
            sln = true;
        }
        else if(flag == "--xcode"){
            xcode = true;
        }
    };

    autom::Gen *gen;

    auto currentDir = std::filesystem::current_path().string();

    autom::OutputTargetOpts outputTargetOpts {targetOS,targetArch,targetPlatform};

    if(!std::filesystem::exists(outputDir.data()))
        std::filesystem::create_directories(outputDir.data());

    if(ninja){
        autom::GenNinjaOpts ninjaOpts {false};
        gen = autom::TargetNinja(outputTargetOpts,ninjaOpts);
    }
    
    auto file = (!toolchainFile.empty())? toolchainFile : std::filesystem::path(exec_path).append("bin").append("default_toolchains.json").string();
    
    autom::ExecEngineOpts opts {
        {defines.data(),defines.data() + defines.size()},
        outputDir,
        *gen,
        file,
        interfaceSearchPaths};
    
    autom::ExecEngine eng(opts,outputTargetOpts);

    auto entry_exists = std::filesystem::exists("./AUTOM.build");
    if(!entry_exists){
        std::cout << "\x1b[31mERROR:\x1b[0m" << "AUTOM.build file not found in current directory!\nExiting..." << std::endl;
        return -1;
    };

    std::ifstream in("./AUTOM.build");
    
    auto startTime = std::chrono::steady_clock::now();

    auto success = eng.parseAndEvaluate(&in);
    if(success) {
        if(eng.checkDependencyTree()){
            eng.generate();
            eng.report();
            auto endTime = std::chrono::steady_clock::now();
            std::chrono::duration<double,std::milli> duration = endTime - startTime;
            std::cout << "Time elapsed:" << duration.count() << "ms" << std::endl;
        };
    }
    else {
        std::cout << "Exiting..." << std::endl;
    }

    return 0;
};
