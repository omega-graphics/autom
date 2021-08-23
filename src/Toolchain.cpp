#include "Toolchain.h"
#include "Toolchains.def"

#include "Diagnostic.h"


#include <fstream>
#include <iostream>

#ifdef _WIN32
#define PATH_ENV_VAR "Path"
#else 
#define PATH_ENV_VAR "PATH"
#endif

namespace autom {

   


    Toolchain::Toolchain():formatter(*this){

    }

    bool Toolchain::verifyTools() const {
        
        std::string path = std::getenv(PATH_ENV_VAR); 
        std::string loc;
        bool res = false;

        #define FIND_TOOL(tool_str) \
            std::cout << "-- " << tool_str << std::endl;\
            if((res = locateProgram(tool_str,path,loc))){\
                std::cout << "-- " << tool_str << "-- found" << " (\"" << loc << "\")";\
            }\
            else {\
                std::cout << "-- " << tool_str << "-- not found";\
            }

        switch (toolchainType) {
            case TOOLCHAIN_CFAMILY_ASM : {

                std::cout << "Toolchain: " << name << std::endl;

                
                FIND_TOOL(CC.command);
                FIND_TOOL(CXX.command);
                FIND_TOOL(AR.command);
                FIND_TOOL(SO_LD.command);
                FIND_TOOL(EXE_LD.command);

                return res;

                break;
            }
        }
    };

    Toolchain::Formatter::Formatter(Toolchain &_toolchain):toolchain(_toolchain) {

    }

    void Toolchain::Formatter::startCommandFormat(Toolchain::Formatter::CommandFormatType type) {
        str.str("");
        _type = type;
    }

    void Toolchain::Formatter::writeCommandPrefix() {
        if(_type == cc || _type == objc){
            str << toolchain.CC.command;
        }
        else if(_type == cxx || _type == objcxx){
            str << toolchain.CXX.command;
        }
        else if(_type == so){
            str << toolchain.SO_LD.command;
        }
        else if(_type == ar){
            str << toolchain.AR.command;
        }
        else if(_type == exe){
            str << toolchain.EXE_LD.command;
        }
    }

    void Toolchain::Formatter::endCommandFormat(std::ostream &out) {
        out << str.str();
    }

    void Toolchain::Formatter::writeSource(const StrRef &src) {
        str << toolchain.compile << " " << src.data();   
    }

    void Toolchain::Formatter::writeFlags(ArrayRef<std::string> flags) {
        for(auto & flag : flags){
            str << flag << " ";
        }
    }

    void Toolchain::Formatter::writeIncludes(ArrayRef<std::string> inc) {
        for(auto & i : inc){
            str << toolchain.include_dir << i << " ";
        }
    }

    void Toolchain::Formatter::writeOutput(const StrRef &output) {
        str << toolchain.output << " " << output.data();
    }

    void Toolchain::Formatter::writeDefines(ArrayRef<std::string> defines) {
        for(auto & d : defines){
            str << toolchain.define << d << " ";
        }
    }

    void Toolchain::Formatter::writeLibs(ArrayRef<std::string> libs) {
        for(auto & l : libs){
            str << toolchain.lib << l << " ";
        }
    }

    void Toolchain::Formatter::writeString(const std::string &str) {
        this->str << str;
    }


ToolchainLoader::ToolchainLoader(const StrRef & path):filename(path){
    std::ifstream in(path,std::ios::in);
    auto inW = rapidjson::IStreamWrapper(in);
    toolchainFile.ParseStream(inW);
}

std::shared_ptr<Toolchain> ToolchainLoader::getToolchain(ToolchainSearchOpts & opts){
    
    auto & document = toolchainFile;

    auto t = new Toolchain;

    if(document.IsArray()){
        auto array = document.GetArray();
        for(auto array_it = array.Begin();array_it != array.End();array_it++){
            auto entry = array_it->GetObject();

            auto toolchain_entry_type = entry.FindMember("type");
            if(toolchain_entry_type == entry.MemberEnd()){
                std::cout << formatmsg("No member found for toolchain entry by the name: `@0`, in file @1","type",filename).res;
                delete t;
                exit(1);
            }

            auto toolchain_entry_platforms = entry.FindMember("platforms");
            if(toolchain_entry_platforms != entry.MemberEnd()){
                auto platforms = toolchain_entry_platforms->value.GetArray();
                for(auto p_it = platforms.Begin();p_it != platforms.End();p_it++){
                    if((std::strcmp(p_it->GetString(),"windows") == 0) && opts.platform == TargetPlatform::Windows){

                    }
                    else if((std::strcmp(p_it->GetString(),"mac") == 0)){

                    }
                }
            }


            autom::StrRef type = toolchain_entry_type->value.GetString();

            auto progs = entry["progs"].GetObject();
            auto flags = entry["flags"].GetObject();


            {
                t->compile = flags["compile"].GetString();
                t->compile = flags["output"].GetString();
            }

            if(type == "cfamily" && opts.type == ToolchainSearchOpts::ccAsmFamily){
                t->toolchainType = TOOLCHAIN_CFAMILY_ASM;
                t->define = flags["define"].GetString();
                t->include_dir = flags["include_dir"].GetString();
                t->lib = flags["lib"].GetString();
                t->lib_dir = flags["lib_dir"].GetString();

                t->CC.command = progs["cc"].GetString();
                t->CXX.command = progs["cxx"].GetString();
                t->AR.command = progs["ar"].GetString();
                t->EXE_LD.command = progs["ld_exe"].GetString();
                t->SO_LD.command = progs["ld_so"].GetString();


            }
            else if(type == "jdk" && opts.type == ToolchainSearchOpts::jdk){
                t->toolchainType = TOOLCHAIN_JDK;
            }

            t->name = entry["name"].GetString();

            if(opts.preferedToolchain.size() != 0){
                
                if(opts.preferedToolchain == t->name){
                    return std::shared_ptr<Toolchain>(t);
                    break;
                }
                else {
                    continue;
                }
            }

            return std::shared_ptr<Toolchain>(t);
            break;
            
        }
    }
    delete t;

    return nullptr;
};


}
