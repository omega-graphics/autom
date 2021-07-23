#include <vector>
#include <string>

#include "Targets.def"

#ifndef AUTOM_TARGET_H
#define  AUTOM_TARGET_H


namespace autom {

    struct Target {
        TargetType type;
        std::string name;
        std::vector<std::string> deps;
    };



    class TargetConsumer {
    public:
        virtual void consumeTarget(Target *target) = 0;
    };

    
};

#endif