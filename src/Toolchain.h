#include "ADT.h"

#ifndef AUTOM_TOOLCHAIN_H
#define AUTOM_TOOLCHAIN_H

namespace autom {



    struct Toolchain {
    private:
        int _toolchain_type = 0;
    public:
        struct Tool {
            std::string command;
        } 
        CC,
        CXX,
        AR,
        SO_LD,
        EXE_LD,
        JAVAC,
        SWIFTC;

        class Formatter {
            typedef enum : int {
                cc,
                cxx,
                objc,
                objcxx,
                swift,
                java
            } CommandFormatType;

            void startCommandFormat(CommandFormatType type);

            void writeCompilerPrefix();

            void writeSource(const autom::StrRef & src);

            void setOutputProps(const autom::StrRef & name,const autom::StrRef & dir);

            void writeFlags(ArrayRef<std::string> flags);

            void writeIncludes(ArrayRef<std::string> inc);

            void endCommandFormat(std::ostringstream &out);
        };
        Formatter formatter;
    };

}

#endif

