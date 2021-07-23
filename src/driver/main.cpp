#include "../Parser.h"
#include "../ADT.h"
#include "../Gen.h"

#include <iostream>
#include <filesystem>
#include <fstream>


inline void printHelp(){
    std::cout << 
R"(AUTOM Build Tool
================
Usage: autom  [options] [output-dir] [AUTOM.build]
--------------------------------------------------

Options:


--help, -h                 --> Show this message

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

    for(unsigned i = 0;i < argc;i++){
        autom::StrRef flag(argv[i]);
        if(flag == "--help" || flag == "-h"){
            printHelp();
        };
    };

    
    auto ninja = autom::TargetNinja();
    autom::Parser parser(*ninja);

    auto entry_exists = std::filesystem::exists("./AUTOM.build");
    if(!entry_exists){
        std::cerr << "\x1b[31mERROR:\x1b[0m" << "AUTOM.build file not found in current directory!\nExiting..." << std::endl;
    };

    std::ifstream in("./AUTOM.build");

    parser.parseAndEvaluate(&in);

    return 0;
};