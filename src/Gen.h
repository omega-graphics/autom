#include "Target.h"
#include "Toolchain.h"
#include "InstallFile.h"

#include <memory>
#include <fstream>

#ifndef AUTOM_GEN_H
#define AUTOM_GEN_H

namespace autom {


    struct ToolchainDefaults {
        eval::Array *c_flags;
        eval::Array *cxx_flags;
    };

    
    class Gen : public TargetConsumer,
                public InstallFileSerializer {
    protected:
        GenContext *context = nullptr;
    public:
        using TargetConsumer::consumeTarget;
        virtual void configGenContext() = 0;
        void setGenContext(GenContext & context){
            this->context = &context;
            configGenContext();
        };
        virtual void consumeToolchainDefaults(ToolchainDefaults & conf) = 0;
        virtual bool supportsCustomToolchainRules() = 0;
        virtual void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) = 0;
        void beginConsumeInstallRules(){
            auto install_file = std::filesystem::path(context->outputDir.data()).append("AUTOMINSTALL").string();
            beginWrite(install_file);
        };
        void consumeInstallRule(InstallRulePtr rule){
            writeRule(rule);
        };
        void endConsumeInstallRules(){
            endWrite();
        };
        virtual void finish() = 0;
    };

    Gen *TargetCompileCommands(OutputTargetOpts &outputOpts,autom::StrRef output_path);

    struct GenNinjaOpts {
        bool splitTargetsByNinja;
    };

    Gen *TargetNinja(OutputTargetOpts &outputOpts,GenNinjaOpts & opts);

////    struct GenCMake {
////
////    };
//
//    Gen *TargetCMake();

    struct GenVisualStudioOpts {
//        autom::StrRef projectName;
//        autom::StrRef outputDir;
    };

    Gen *TargetVisualStudio(OutputTargetOpts & outputOpts,GenVisualStudioOpts & opts);
    
    struct GenXcodeOpts {
//        autom::StrRef projectName;
//        autom::StrRef outputDir;
    };

    Gen *TargetXcode(OutputTargetOpts & outputOpts,GenXcodeOpts & opts);

    Gen *TargetGradle();

};

#endif
