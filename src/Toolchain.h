#include "ADT.h"

#ifndef AUTOM_TOOLCHAIN_H
#define  AUTOM_TOOLCHAIN_H

namespace autom {

    struct Toolchain {
        struct Tool {
            std::string command;
        } 
        CC,
        CXX,
        AR,
        SO_LD,
        EXE_LD;

    };

}

#endif

