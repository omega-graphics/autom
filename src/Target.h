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

    const char *TargetOSToString(TargetOS &os);

    enum class TargetPlatform : int {
        macOS,
        iOS,
        Android,
        Linux,
        Windows
    };

    const char *TargetPlatformToString(TargetPlatform &platform);

    enum class TargetArch : int {
        ARM,
        AARCH64,
        x86,
        x86_64
    };

    const char *TargetArchToString(TargetArch & arch);

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
        eval::Array *deps = new eval::Array();
        
        std::vector<Target *> resolvedDeps;
        
    };

    struct GroupTarget : public Target {
        GroupTarget(){
            type = GROUP_TARGET;
        }
        static GroupTarget *Create(eval::String *name,eval::Array *deps){
            auto t = new GroupTarget();
            t->name = name;
            t->deps = deps;
            return t;
        };
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
        eval::Array * lib_dirs;

        /// type = string[]
        eval::Array * defines;

        /// type = string[]
        eval::Array * include_dirs;

        CompiledTarget(){
            cflags = new eval::Array();
            libs = new eval::Array();
            lib_dirs = new eval::Array();
            defines = new eval::Array();
            include_dirs = new eval::Array();
            ldflags = new eval::Array();
            output_ext = new eval::String();
        }

        static CompiledTarget * Executable(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget();
            t->name = name;
            t->srcs = sources;
            t->type = EXECUTABLE;
            return t;
        };

        static CompiledTarget * Archive(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget();
            t->name = name;
            t->srcs = sources;
            t->type = STATIC_LIBRARY;
            return t;
        };

        static CompiledTarget * Shared(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget();
            t->name = name;
            t->srcs = sources;
            t->type = SHARED_LIBRARY;
            return t;
        };
        
        static CompiledTarget * SourceGroup(eval::String * name,eval::Array * sources){
            auto * t = new CompiledTarget();
            t->name = name;
            t->srcs = sources;
            t->type = SOURCE_GROUP;
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
