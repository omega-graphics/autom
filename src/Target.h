#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "Targets.def"

#include "ADT.h"

#ifndef AUTOM_TARGET_H
#define AUTOM_TARGET_H


namespace autom {

    /// @name Output Target Options
    /// @{

    enum class TargetOS : int {
        Darwin,
        Linux,
        Windows
    };

    enum class TargetPlatform : int {
        macOS,
        iOS,
        Android,
        Linux,
        Windows
    };

    enum class TargetArch : int {
        ARM,
        AARCH64,
        x86,
        x86_64
    };

    struct OutputTargetOpts {
        TargetOS os;
        TargetArch arch;
        TargetPlatform platform;
    };

    TargetArch hostArch();
    TargetOS hostOS();
    TargetPlatform hostPlatform();

    /// @}


    /// @name Source Targets
    /// @{

    struct Target {
        TargetType type = 0;
        std::string name;
        std::vector<std::string> deps;
    };

    struct CompiledTarget : public Target {

        std::unordered_map<std::string,std::string> source_object_map;

        /// Other Object files to link into target

        std::vector<std::string> other_objs;

        std::string output_ext;

        std::vector<std::string> cflags;

        std::vector<std::string> ldflags;

        std::vector<std::string> libs;

        std::vector<std::string> defines;

        std::vector<std::string> include_dirs;

        static CompiledTarget * Executable(autom::StrRef name,autom::ArrayRef<std::string> sources){
            auto * t = new CompiledTarget;
            t->name = name;
            t->type = EXECUTABLE;
            for(auto & s : sources){
                t->source_object_map.insert(std::make_pair(s,""));
            }
            return t;
        };

        static CompiledTarget * Archive(autom::StrRef name,autom::ArrayRef<std::string> sources){
            auto * t = new CompiledTarget;
            t->name = name;
            t->type = STATIC_LIBRARY;
            for(auto & s : sources){
                t->source_object_map.insert(std::make_pair(s,""));
            }
            return t;
        };

        static CompiledTarget * Shared(autom::StrRef name,autom::ArrayRef<std::string> sources){
            auto * t = new CompiledTarget;
            t->name = name;
            t->type = SHARED_LIBRARY;
            for(auto & s : sources){
                t->source_object_map.insert(std::make_pair(s,""));
            }
            return t;
        };

    };


    

    



    class TargetConsumer {
    public:
        virtual void consumeTarget(Target *target) = 0;
    };

    /// @}

    
};

#endif