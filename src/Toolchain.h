#include "ADT.h"
#include <string>
#include <sstream>
#include <regex>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "Target.h"

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

    struct ToolchainSearchOpts {
        StrRef preferedToolchain;
        bool stubborn;
        typedef enum : int {
            ccAsmFamily,
            jdk,
            swiftFamily
        } ToolchainTy;
        ToolchainTy type;
    };

    struct Toolchain {
        int toolchainType = 0;
        std::string name;
    public:
        Toolchain();
        struct Tool {
            std::string command;
        } 
        /// @name Tools
        /// @{
        CC,
        CXX,
        AR,
        SO_LD,
        EXE_LD,
        JAVAC,
        SWIFTC;
        /// @}

        typedef std::string Flag;

        /// @name Flags
        /// @{
        Flag define;
        Flag include_dir;
        Flag lib;
        Flag lib_dir;
        Flag compile;
        Flag output;
        /// @}

        bool verifyTools() const;


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


    class ToolchainLoader {
        rapidjson::Document toolchainFile;
        StrRef filename;
    public:
        ToolchainLoader(const StrRef & path);
        
        std::shared_ptr<Toolchain> getToolchain(ToolchainSearchOpts & opts);
        
    };

}

#endif

