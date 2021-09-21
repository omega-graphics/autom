#include "AST.def"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>

#ifndef AUTOM_ENGINE_AST_H
#define  AUTOM_ENGINE_AST_H


namespace autom {

    struct ASTScope {
        std::string name;
        ASTScope *parent;
 
        
        bool refCount = 1;
        /** 
         @brief Recursively checks scope inheritance.
         @param parent The ASTScope used to check for inheritance.
         @returns bool
         */
        bool isChildScopeOfParent(ASTScope *parent);
    };

    
    ASTScope *ASTScopeCreate(const char *name,ASTScope *parent);
    void ASTScopeAddReference(ASTScope *scope);
    void ASTScopeRelease(ASTScope *scope);

    extern ASTScope *GLOBAL_SCOPE;

    struct ASTNode {
        ASTNodeType type;
        ASTScope *scope;
        ~ASTNode();
    };

    

    struct ASTExpr : public ASTNode {

        ASTExpr *lhs,*rhs;

        std::string id;
        
        std::string operand;

        std::vector<ASTExpr *> children;

        std::unordered_map<std::string,ASTExpr *> func_args;
        
    };

    struct ASTLiteral : public ASTExpr {
        std::optional<std::string> str;

        std::optional<bool> boolean;

        bool isString();
        bool isBoolean();
    };

    struct ASTBlock : public ASTNode {
         std::vector<ASTNode *> body;
    };

    struct ASTImportDecl : public ASTNode {
        bool isInterface;
        std::string value;
    };

    struct ASTForeachDecl : public ASTNode {
        std::string loopVar;
        ASTExpr *list;
        std::unique_ptr<ASTBlock> body;
    };

    struct ASTConditionalDecl : public ASTNode {
        struct CaseSpec {
            bool testCondition;
            ASTExpr *condition;
            std::unique_ptr<ASTBlock> block;
        };
        std::vector<CaseSpec> cases;
    };

    struct ASTFuncDecl : public ASTNode {
        std::string id;
        std::vector<std::string> params;
        std::unique_ptr<ASTBlock> body;
    };

    struct ASTVarDecl : public ASTNode {
        std::string id;
        std::optional<ASTExpr *> init;
    };

    
};

#endif
