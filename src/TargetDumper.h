#include "Target.h"

#ifndef AUTOM_TARGET_DUMPER_H
#define  AUTOM_TARGET_DUMPER_H

namespace autom {
    /**
     @brief Dumps Target Info to Output Stream. Useful for debugging
    */
    class TargetDumper : public TargetConsumer {
        std::ostream & out;
    public:
        TargetDumper(std::ostream & out);
        void consumeTarget(Target *target) override;
    };

}

#endif 