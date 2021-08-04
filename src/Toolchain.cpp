#include "Toolchain.h"
#include "Toolchains.def"

namespace autom {

    Toolchain::Toolchain():formatter(*this){

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
        switch(toolchain.toolchainType){
            case TOOLCHAIN_MSVC : {
                str << "/C" << src.data();
            }
            default : {
                str << "-c" << " " << src.data();
            }
        }
    }

    void Toolchain::Formatter::writeFlags(ArrayRef<std::string> flags) {
        for(auto & flag : flags){
            str << flag << " ";
        }
    }

    void Toolchain::Formatter::writeIncludes(ArrayRef<std::string> inc) {
        std::string prefix;
        if(toolchain.toolchainType == TOOLCHAIN_MSVC || toolchain.CC.command == LLVM_CLANGMSVC){
            prefix = "/I";
        }
        else {
            prefix = "-I";
        }

        for(auto & i : inc){
            str << prefix << i << " ";
        }
    }

    void Toolchain::Formatter::writeOutput(const StrRef &output) {

        switch(toolchain.toolchainType){
            case TOOLCHAIN_MSVC : {
                str << "/Fo" << output.data();
            }
            default : {
                str << "-o" << " " << output.data();
            }
        }

    }

    void Toolchain::Formatter::writeDefines(ArrayRef<std::string> defines) {
        std::string prefix;
        switch(toolchain.toolchainType){
            case TOOLCHAIN_MSVC : {
                prefix = "/D";
            }
            case TOOLCHAIN_LLVM | TOOLCHAIN_GCC : {
                prefix = "-D";
            }
        }
        for(auto & d : defines){
            str << prefix << d << " ";
        }
    }

    void Toolchain::Formatter::writeLibs(ArrayRef<std::string> libs) {
        std::string prefix;
        switch(toolchain.toolchainType){
            case TOOLCHAIN_MSVC : {
                prefix = "";
            }
            case TOOLCHAIN_LLVM | TOOLCHAIN_GCC : {
                prefix = "-l";
            }
        }
        for(auto & l : libs){
            str << prefix << l << " ";
        }
    }

    void Toolchain::Formatter::writeString(const std::string &str) {
        this->str << str;
    }


}