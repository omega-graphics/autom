#include "engine/Execution.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>

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
        
        std::vector<std::shared_ptr<Target>> resolvedDeps;
        virtual ~Target() = default;
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

        /// @note private use for Windows only.
        /// type = string
        eval::String * implib_ext;
        
        /// type = string
        eval::String * output_dir;

        /// type = string[]
        eval::Array * cflags;

        /// type = string[]
        eval::Array * ldflags;

        /// type = string[]
        eval::Array * libs;
        
        /// type = string[]
        eval::Array * lib_dirs;
        
#ifdef __APPLE__
        
        /// type = string[]
        eval::Array * frameworks;
        
        /// type = string[]
        eval::Array * framework_dirs;
        
#endif

        /// type = string[]
        eval::Array * defines;

        /// type = string[]
        eval::Array * include_dirs;

        CompiledTarget(){
            cflags = new eval::Array();
            libs = new eval::Array();
            lib_dirs = new eval::Array();
#ifdef __APPLE__
            frameworks = new eval::Array();
            framework_dirs = new eval::Array();
#endif
            defines = new eval::Array();
            include_dirs = new eval::Array();
            ldflags = new eval::Array();
            output_ext = new eval::String();
            output_dir = new eval::String();
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


    struct JavaTarget : public Target {
        
        /// type = string
        eval::String * src_dir;
        
        /// type = string[]
        eval::Array * maven_deps;
        
        /// type = string[]
        eval::Array * maven_repos;
        
        JavaTarget(){
            maven_deps = new eval::Array();
            maven_repos = new eval::Array();
        };
        
        static JavaTarget *JarLib(eval::String * name,eval::String *src_dir){
            auto * t = new JavaTarget();
            t->type = JAR_LIB;
            t->name = name;
            t->src_dir = src_dir;
            return t;
        }
        
        static JavaTarget *JarExe(eval::String * name,eval::String *src_dir){
            auto * t = new JavaTarget();
            t->type = JAR_EXE;
            t->name = name;
            t->src_dir = src_dir;
            return t;
        }
    };

    struct FSTarget : public Target {
        eval::Array * sources = nullptr;
        
        eval::String * dest = nullptr;
        
        eval::String *symlink_src = nullptr;
        
        static FSTarget * Copy(eval::String *name,eval::Array * sources,eval::String * dest){
            auto *t = new FSTarget();
            t->type = FS_COPY;
            t->name = name;
            t->sources = sources;
            t->dest = dest;
            return t;
        }
        
        static FSTarget * Symlink(eval::String *name,eval::String * source,eval::String * dest){
            auto *t = new FSTarget();
            t->type = FS_SYMLINK;
            t->name = name;
            t->symlink_src = source;
            t->dest = dest;
            return t;
        }
        
        static FSTarget * Mkdir(eval::String *name,eval::String * dest){
            auto *t = new FSTarget();
            t->type = FS_MKDIR;
            t->name = name;
            t->dest = dest;
            return t;
        }
    };


    struct ScriptTarget : public Target {
        eval::String * script;
        
        eval::String * desc;
        
        eval::Array *args;
        
        eval::Array *outputs;
        
        static ScriptTarget *Create(eval::String * name,eval::String *script,eval::Array * args,eval::Array *outputs){
            auto *t = new ScriptTarget();
            t->type = SCRIPT_TARGET;
            t->name = name;
            t->script = script;
            t->desc = new eval::String();
            t->desc->assign(name);
            t->args = args;
            t->outputs = outputs;
            return t;
        }
    };


    

    



    class TargetConsumer {
    public:
        virtual void consumeTarget(std::shared_ptr<Target> & target) = 0;
    };

    /// @}

    
}

#endif
