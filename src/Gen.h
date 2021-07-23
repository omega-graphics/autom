#include "Target.h"

#ifndef AUTOM_GEN_H
#define AUTOM_GEN_H

namespace autom {

    struct GenNinjaOpts {
        
    };

    TargetConsumer *TargetNinja();

    TargetConsumer *TargetCMake();

    TargetConsumer *TargetVisualStudio();

    TargetConsumer *TargetXcode();

};

#endif