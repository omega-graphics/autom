#include "TargetDumper.h"
#include <iostream>

namespace autom {
    TargetDumper::TargetDumper(std::ostream & out):t_count(0),out(out){

    };

    void TargetDumper::consumeTarget(Target *target){
        std::cout << "Target " << t_count << std::hex << target->type << std::dec << std::endl;
    };

    
}