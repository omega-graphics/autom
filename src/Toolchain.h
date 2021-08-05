#include "ADT.h"
#include <string>
#include <sstream>
#include <regex>

#ifndef AUTOM_TOOLCHAIN_H
#define AUTOM_TOOLCHAIN_H

namespace autom {

#define SOURCE_MATCHER(name,reg) \
inline bool name(StrRef subject){ \
        std::regex r(reg,std::regex_constants::ECMAScript);\
        return std::regex_search(subject.data(),r);\
        };

    SOURCE_MATCHER(isCSrc,R"(\.c)")
    SOURCE_MATCHER(isCXXSrc,R"(\.(?:cc|cxx|cpp))")
    SOURCE_MATCHER(isOBJCSrc,R"(\.m)")
    SOURCE_MATCHER(isOBJCXXSrc,R"(\.mm)")

    struct Toolchain {
        int toolchainType = 0;
    public:
        Toolchain();
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
            std::ostringstream str;
            Toolchain &toolchain;
        public:
            typedef enum : int {
                unknown,
                cc,
                cxx,
                so,
                ar,
                exe,
                objc,
                objcxx,
                swift,
                java
            } CommandFormatType;
        private:
            CommandFormatType _type;
        public:
            explicit Formatter(Toolchain & _toolchain);
            void startCommandFormat(CommandFormatType type);

            void writeCommandPrefix();

            void writeString(const std::string & str);

            void writeSource(const autom::StrRef & src);

            void writeOutput(const StrRef &output);

            void writeFlags(ArrayRef<std::string> flags);

            void writeDefines(ArrayRef<std::string> defines);

            void writeLibs(ArrayRef<std::string> libs);

            void writeIncludes(ArrayRef<std::string> inc);

            void endCommandFormat(std::ostream &out);
        };
        friend class Formatter;
        Formatter formatter;
    };

}

#endif

