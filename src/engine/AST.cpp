#include "AST.h"

namespace autom {


    ASTScope *GLOBAL_SCOPE = new ASTScope{"GLOBAL",nullptr};

    bool ASTLiteral::isBoolean(){
        return boolean.has_value();
    };

    bool ASTLiteral::isString(){
        return str.has_value();
    };
};