#include "Target.h"
#include "Toolchain.h"

#ifndef AUTOM_GEN_H
#define AUTOM_GEN_H

namespace autom {
    
    class Gen : public TargetConsumer {
    public:
        using TargetConsumer::consumeTarget;
        virtual bool supportsCustomToolchainRules() = 0;
        virtual void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) = 0;
        virtual void finish() = 0;
    };

    Gen *TargetCompileCommands(OutputTargetOpts &outputOpts,autom::StrRef output_path);

    struct GenNinjaOpts {
        autom::StrRef outputDir;
        autom::StrRef srcDir;
        bool splitTargetsByNinja;
    };

    Gen *TargetNinja(OutputTargetOpts &outputOpts,GenNinjaOpts & opts);

//    struct GenCMake {
//
//    };

    Gen *TargetCMake();

    struct GenVisualStudioOpts {
        autom::StrRef projectName;
        autom::StrRef outputDir;
    };

    Gen *TargetVisualStudio(OutputTargetOpts & outputOpts,GenVisualStudioOpts & opts);
    
    struct GenXcodeOpts {
        autom::StrRef projectName;
        autom::StrRef outputDir;
    };

    Gen *TargetXcode(OutputTargetOpts & outputOpts,GenXcodeOpts & opts);

};

#endif
