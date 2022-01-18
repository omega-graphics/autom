
#include <filesystem>
#include <fstream>
#include <iostream>

#include "ADT.h"

#include "InstallFile.h"

inline auto printHelp(){
    std::cout <<
    R"(Usage: autom-install [options] build_dir install_prefix
Options:
    
    --help, -h  --> Show this message
    )" << std::endl;
    exit(0);
};

class InstallDriver : public autom::InstallFileSerializer {
public:
    void performInstallation(const autom::StrRef & path,const autom::StrRef & prefix){
        beginRead(path);
        autom::InstallRulePtr rule;
        auto outputDir = std::filesystem::path(path.data()).parent_path();
        while(getRule(rule)){
            auto p = std::filesystem::path(prefix.data()).append(rule->prefixed_dest.data());
            if(!std::filesystem::exists(p)){
                std::filesystem::create_directories(p);
            }
            if(rule->type == autom::InstallRule::Target){
                auto _t = std::dynamic_pointer_cast<autom::TargetInstallRule>(rule);
                std::cout << "- Installing targets -->" << std::endl << "       ";
                for(auto & t : _t->targets){
                    auto f_name = std::filesystem::path(t->name->value().data()).filename();
                    std::filesystem::copy(std::filesystem::path(outputDir).append(t->name->value().data()),std::filesystem::path(prefix.data()).append(rule->prefixed_dest).append(f_name.string()).string(),std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive);
                    std::cout << "- " << t->name->value() << std::endl << "       ";
                }
                
            }
            else {
                auto _t = std::dynamic_pointer_cast<autom::FileInstallRule>(rule);
                std::cout << "- Installing files -->" << std::endl << "       ";
                
                for(auto & f : _t->files){
                    auto p = std::filesystem::path(f);
                    std::cout << "- " << p.string() << std::endl << "       ";
                    if(std::filesystem::is_directory(p)){
                        std::filesystem::copy(p,std::filesystem::path(prefix.data()).append(rule->prefixed_dest).append(p.filename().string()),
                                              std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
                    }
                    else {
                        std::filesystem::copy(p,std::filesystem::path(prefix.data()).append(rule->prefixed_dest).append(p.filename().string()),std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive);
                    }
                }
                
            }
            std::cout << std::endl;
        }
        endRead();
        
        
    }
};

int main(int argc,char *argv[]){
    
   
    --argc;
    
    auto c = argc;
    
    
    unsigned i = 1;
    for(;c > 0;--c){
        autom::StrRef flag{argv[i]};
        
        if(flag == "--help" || flag == "-h"){
            printHelp();
            break;
        }
        
        i++;
    };
    
    if(argc < 2){
        std::cout << "ERROR: Expected a minimum of 2 arguments" << std::endl;
        return -1;
    };
    
    autom::StrRef buildDir {argv[1]};
    autom::StrRef installPrefix {argv[2]};
    
    auto autom_install_file = std::filesystem::path(buildDir.data()).append("AUTOMINSTALL");
    
    if(!std::filesystem::exists(autom_install_file)){
        std::cout << "ERROR: AUTOMINSTALL not found in" << buildDir << "\nExiting..." << std::endl;
        return -1;
    };
    
    if(!std::filesystem::exists(installPrefix.data())){
        std::filesystem::create_directories(installPrefix.data());
    }
    
    InstallDriver driver;
    driver.performInstallation(autom_install_file.string(),installPrefix);
    
    
    return 0;
};
