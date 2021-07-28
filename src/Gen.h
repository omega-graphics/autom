#include "Target.h"

#ifndef AUTOM_GEN_H
#define AUTOM_GEN_H

namespace autom {
    
    class Gen : public TargetConsumer {
    public:
        using TargetConsumer::consumeTarget;
        virtual bool supportsCustomToolchainRules() = 0;
        virtual void genToolchainRules() = 0;
        virtual void finish() = 0;
    };

    struct GenNinjaOpts {
        
    };

    Gen *TargetNinja();

    Gen *TargetCMake();

    Gen *TargetVisualStudio();

    Gen *TargetXcode();

};

#endif