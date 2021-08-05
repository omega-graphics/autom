#include "AST.h"

namespace autom {


    ASTScope *GLOBAL_SCOPE = new ASTScope{"GLOBAL",nullptr};

    bool ASTScope::isChildScopeOfParent(ASTScope *parent){
        if(this->parent == parent)
            return true;

        while(!parent){
            parent = parent->parent;
            if(this->parent == parent){
                return true;
            }
        }
        return false;
    };

    bool ASTLiteral::isBoolean(){
        return boolean.has_value();
    };

    bool ASTLiteral::isString(){
        return str.has_value();
    };
};