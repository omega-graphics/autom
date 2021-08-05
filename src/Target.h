#include "engine/Execution.h"

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

        /// type = string
        eval::String *name;

        /// type = string[]
        eval::String *deps;
    };

    struct CompiledTarget : public Target {

        /// type = string[]
        eval::Array * srcs;

        std::unordered_map<std::string,std::string> source_object_map;

        /// Other Object files to link into target

        std::vector<std::string> other_objs;

        /// type = string
        eval::String * output_ext;

        /// type = string[]
        eval::Array * cflags;

        /// type = string[]
        eval::Array * ldflags;

        /// type = string[]
        eval::Array * libs;

        /// type = string[]
        eval::Array * defines;

        /// type = string[]
        eval::Array * include_dirs;

        static CompiledTarget * Executable(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget;
            t->name = name;
            t->srcs = sources;
            t->type = EXECUTABLE;
            return t;
        };

        static CompiledTarget * Archive(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget;
            t->name = name;
            t->srcs = sources;
            t->type = STATIC_LIBRARY;
            return t;
        };

        static CompiledTarget * Shared(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget;
            t->name = name;
            t->srcs = sources;
            t->type = SHARED_LIBRARY;
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