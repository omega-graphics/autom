#include "Target.h"
#include "Gen.h"

#ifndef AUTOM_TARGET_DUMPER_H
#define  AUTOM_TARGET_DUMPER_H

namespace autom {
    /**
     @brief Dumps Target Info to Output Stream. Useful for debugging
    */
    class TargetDumper : public Gen {
        unsigned t_count;
        std::ostream & out;
    public:
        explicit TargetDumper(std::ostream & out);
        void consumeTarget(std::shared_ptr<Target> & target) override;
        void configGenContext() override;
        bool supportsCustomToolchainRules() override {return false;};
        void genToolchainRules(std::shared_ptr<Toolchain> &toolchain) override {};
        void finish() override{};
    };

}

#endif 
