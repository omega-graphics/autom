#include "Toolchain.h"
#include "Toolchains.def"

#include "Diagnostic.h"

#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>

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

        #define FIND_TOOL(tool_str,tool_name) \
            std::cout << (tool_name) << " --> " << (tool_str) << std::endl;\
            if((res = locateProgram(tool_str,path,loc))){\
                std::cout << "-- " << (tool_str) << " -- found" << " (\"" << loc << "\")" << std::endl;\
            }\
            else {\
                std::cout << "-- " << (tool_str) << "-- not found" << std::endl;\
            }

        switch (toolchainType) {
            case TOOLCHAIN_CFAMILY_ASM : {

                std::cout << "Toolchain: " << name << std::endl;

                
                FIND_TOOL(CC.command,"C Compiler");
                FIND_TOOL(CXX.command,"C++ Compiler");
#ifdef __APPLE__
                FIND_TOOL(OBJC.command,"ObjC Compiler");
                FIND_TOOL(OBJCXX.command,"ObjC++ Compiler");
#endif
                FIND_TOOL(AR.command,"Archive Tool");
                FIND_TOOL(SO_LD.command,"Shared Linker");
                FIND_TOOL(EXE_LD.command,"Executable Linker");

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
        if(_type == cc){
            str << toolchain.CC.command;
        }
        else if(_type == cxx ){
            str << toolchain.CXX.command;
        }
#if defined(__APPLE__)
        else if(_type == objc){
            str << toolchain.OBJC.command;
        }
        else if(_type == objcxx){
            str << toolchain.OBJCXX.command;
        }
#endif
        else if(_type == so){
            str << toolchain.SO_LD.command << " " << toolchain.shared;
        }
        else if(_type == ar){
            str << toolchain.AR.command;
        }
        else if(_type == exe){
            str << toolchain.EXE_LD.command << " " << toolchain.executable;
        }
    }

    void Toolchain::Formatter::endCommandFormat(std::ostream &out) {
        out << str.str();
    }

    void Toolchain::Formatter::writeSource(const StrRef &src) {
        if(_type == cc || _type == cxx || _type == objc || _type == objcxx){
            str << toolchain.compile << " " << src.data();
        }
        else {
            str << " " << src.data();
        }
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
        if(_type == exe || _type == so || _type == ar){
            str << toolchain.link_output << output.data();
        }
        else {
            str << toolchain.compile_output << output.data();
        }
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

    void Toolchain::Formatter::writeLibDirs(ArrayRef<std::string> lib_dirs) {
        for(auto & l : lib_dirs){
            str << toolchain.lib_dir << l << " ";
        }
    }

#ifdef __APPLE__

    void Toolchain::Formatter::writeFrameworks(ArrayRef<std::string> frameworks) {
        for(auto & f : frameworks){
            str << toolchain.framework << " " << f << " ";
        }
    }

    void Toolchain::Formatter::writeFrameworkDirs(ArrayRef<std::string> framework_dirs) {
        for(auto & f : framework_dirs){
            str << toolchain.framework_dir << f << " ";
        }
    }

#endif

    void Toolchain::Formatter::writeString(const std::string &str) {
        this->str << str;
    }



    void cacheToolchain(std::shared_ptr<Toolchain> & toolchain,StrRef file){
        std::ofstream out(file,std::ios::out);
        rapidjson::OStreamWrapper out_w(out);
        rapidjson::PrettyWriter writer(out_w);
        writer.StartObject();
        writer.Key("name",4);
        writer.String(toolchain->name.c_str(),toolchain->name.size());
        writer.Key("type",4);
        
        switch (toolchain->toolchainType) {
            case TOOLCHAIN_CFAMILY_ASM : {
                writer.String("cfamily",7);
                
                writer.Key("progs",4);
                
                writer.StartObject();
                writer.Key("cc",2);
                writer.String(toolchain->CC.command.c_str(),toolchain->CC.command.size());
                writer.Key("cxx",3);
                writer.String(toolchain->CXX.command.c_str(),toolchain->CXX.command.size());
#ifdef __APPLE__
                writer.Key("objc",2);
                writer.String(toolchain->OBJC.command.c_str(),toolchain->OBJC.command.size());
                writer.Key("objcxx",3);
                writer.String(toolchain->OBJCXX.command.c_str(),toolchain->OBJCXX.command.size());
#endif
                writer.Key("ar",2);
                writer.String(toolchain->AR.command.c_str(),toolchain->AR.command.size());
                writer.Key("ld_exe",6);
                writer.String(toolchain->EXE_LD.command.c_str(),toolchain->EXE_LD.command.size());
                writer.Key("ld_so",5);
                writer.String(toolchain->SO_LD.command.c_str(),toolchain->SO_LD.command.size());
                writer.EndObject();
                
                writer.Key("flags",5);
                writer.StartObject();
                writer.Key("define",6);
                writer.String(toolchain->define.c_str(),toolchain->define.size());
                writer.Key("include_dir",11);
                writer.String(toolchain->include_dir.c_str(),toolchain->include_dir.size());
                writer.Key("lib",3);
                writer.String(toolchain->lib.c_str(),toolchain->lib.size());
                writer.Key("lib_dir",7);
                writer.String(toolchain->lib_dir.c_str(),toolchain->lib_dir.size());
#ifdef __APPLE__
                writer.Key("framework",9);
                writer.String(toolchain->framework.c_str(),toolchain->framework.size());
                writer.Key("framework_dir",13);
                writer.String(toolchain->framework_dir.c_str(),toolchain->framework_dir.size());
#endif
                writer.Key("compile",7);
                writer.String(toolchain->compile.c_str(),toolchain->compile.size());
                writer.Key("compile_output",14);
                writer.String(toolchain->compile_output.c_str(),toolchain->compile_output.size());
                writer.Key("link_output",11);
                writer.String(toolchain->link_output.c_str(),toolchain->link_output.size());
                writer.EndObject();
                break;
            }
        }
       
        writer.EndObject();
    }

    bool toolchainHasBeenCached(StrRef file){
        return std::filesystem::exists(file.data());
    };

    std::shared_ptr<Toolchain> fromCacheFile(StrRef file){
        std::ifstream in(file,std::ios::in);
        auto inW = rapidjson::IStreamWrapper(in);
        rapidjson::Document doc;
        doc.ParseStream(inW);

        return nullptr;
        
    };



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
                    bool toolchainSupported = false;
                    auto platforms = toolchain_entry_platforms->value.GetArray();
                    for(auto p_it = platforms.Begin();p_it != platforms.End();p_it++){
                        if((std::strcmp(p_it->GetString(),"windows") == 0) && opts.platform == TargetPlatform::Windows){
                            toolchainSupported = true;
                        }
                        else if((std::strcmp(p_it->GetString(),"macos") == 0) && opts.platform == TargetPlatform::macOS){
                            toolchainSupported = true;
                        }
                    }
                    
                    if(!toolchainSupported){
                        continue;
                    }
                }
                
                


                autom::StrRef type = toolchain_entry_type->value.GetString();

                auto progs = entry["progs"].GetObject();
                auto flags = entry["flags"].GetObject();


                {
                    t->compile = flags["compile"].GetString();
                    t->link_output = flags["link_output"].GetString();
                    t->compile_output = flags["compile_output"].GetString();
                }

                if(type == "cfamily" && opts.type == ToolchainSearchOpts::ccAsmFamily){
                    t->toolchainType = TOOLCHAIN_CFAMILY_ASM;
                    t->define = flags["define"].GetString();
                    t->include_dir = flags["include_dir"].GetString();
                    t->lib = flags["lib"].GetString();
                    t->lib_dir = flags["lib_dir"].GetString();
                    
#ifdef __APPLE__
                    t->framework = flags["framework"].GetString();
                    t->framework_dir = flags["framework_dir"].GetString();
#endif
                    
                    t->stripLibPrefix = flags["strip_lib_prefix"].GetBool();
                    
                    t->shared = flags["shared"].GetString();
                    t->executable = flags["executable"].GetString();

                    t->CC.command = progs["cc"].GetString();
                    t->CXX.command = progs["cxx"].GetString();
#ifdef __APPLE__
                    t->OBJC.command = progs["objc"].GetString();
                    t->OBJCXX.command = progs["objcxx"].GetString();
#endif
                    t->AR.command = progs["ar"].GetString();
                    t->EXE_LD.command = progs["ld_exe"].GetString();
                    t->SO_LD.command = progs["ld_so"].GetString();


                }
//               
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
