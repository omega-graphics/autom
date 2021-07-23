#include "../Gen.h"

namespace autom {

    class GenNinja : public TargetConsumer {

        void consumeTarget(Target *target) override{

        };
        
    };

    TargetConsumer *TargetNinja(){
        return new GenNinja();
    };
};