#include "AST.h"

namespace autom {


    ASTScope *GLOBAL_SCOPE = ASTScopeCreate("__GLOBAL__",nullptr);

    ASTScope *ASTScopeCreate(const char *name,ASTScope *parent){
        return new ASTScope {name,parent,1};
    };

    void ASTScopeAddReference(ASTScope *scope){
        scope->refCount += 1;
    };

    void ASTScopeRelease(ASTScope *scope){
        scope->refCount -= 1;
        if(scope->refCount == 0){
            delete scope;
        };
    };

    bool ASTScope::isChildScopeOfParent(ASTScope *parent){
        ASTScope *par = this->parent;
        while(true){
            if(parent == par){
                return true;
            }
            if(par == nullptr){
                break;
            }
            par = par->parent;
            
        }
        return false;
    };

    ASTNode::~ASTNode(){
        ASTScopeRelease(scope);
    };

    bool ASTLiteral::isBoolean(){
        return boolean.has_value();
    };

    bool ASTLiteral::isString(){
        return str.has_value();
    };
};
