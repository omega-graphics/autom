
#include <filesystem>
#include <fstream>
#include <iostream>

#include "ADT.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

inline auto printHelp(){
    std::cout <<
    R"(Usage: autom-install [options] build_dir install_prefix
Options:
    
    --help, -h  --> Show this message
    )";
    exit(0);
};

void performInstallation(const autom::StrRef & install_file,const autom::StrRef & installDest){
    std::ifstream in(install_file.data(),std::ios::in);
    rapidjson::IStreamWrapper in_json(in);
    rapidjson::Document document;
    document.ParseStream(in_json);
    
    
    
};

int main(int argc,char *argv[]){
    
   
    --argc;
    
    auto c = argc;
    
    if(argc < 2){
        std::cout << "ERROR: Expected a minimum of 2 arguments" << std::endl;
        return -1;
    };
    
    unsigned i = 1;
    for(;argc > 0;--argc){
        autom::StrRef flag{argv[i]};
        
        if(flag == "--help" || flag == "-h"){
            printHelp();
            break;
        }
        
        i++;
    };
    
    autom::StrRef buildDir {argv[argc-1]};
    autom::StrRef installPrefix {argv[argc]};
    
    auto autom_install_file = std::filesystem::path(buildDir.data()).append("AUTOMINSTALL");
    
    if(!std::filesystem::exists(autom_install_file)){
        std::cout << "ERROR: AUTOMINSTALL not found in" << buildDir << "\nExiting..." << std::endl;
        return -1;
    };
    
    if(!std::filesystem::exists(installPrefix.data())){
        std::filesystem::create_directories(installPrefix.data());
    }
    
    
    
    
    return 0;
};
