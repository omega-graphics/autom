#include <vector>
#include <string>

#include "Targets.def"

#include "ADT.h"

#ifndef AUTOM_TARGET_H
#define  AUTOM_TARGET_H


namespace autom {

    struct Target {
        TargetType type;
        std::string name;
        std::vector<std::string> deps;
    };

    struct CompiledTarget : public Target {

        std::vector<std::string> sources;

        std::vector<std::string> cflags;

        std::vector<std::string> ldflags;

        static CompiledTarget * Executable(autom::StrRef name,autom::ArrayRef<std::string> sources){
            auto * t = new CompiledTarget;
            t->type = EXECUTABLE;
            t->sources = sources;
            return t;
        };
    };


    

    



    class TargetConsumer {
    public:
        virtual void consumeTarget(Target *target) = 0;
    };

    
};

#endif