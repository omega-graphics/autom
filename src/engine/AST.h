#include "AST.def"

#include <string>
#include <vector>

#include <unordered_map>
#include <optional>

#ifndef AUTOM_ENGINE_AST_H
#define  AUTOM_ENGINE_AST_H


namespace autom {

    struct ASTScope {
        std::string name;
        ASTScope *parent;
    };

    extern ASTScope *GLOBAL_SCOPE;

    struct ASTNode {
        ASTNodeType type;
        ASTScope *scope;
    };

    

    struct ASTExpr : public ASTNode {

        ASTExpr *lhs,*rhs;

        std::string id;

        std::vector<ASTExpr *> children;

        std::unordered_map<std::string,ASTExpr *> func_args;

        std::unordered_map<ASTExpr *,ASTExpr *> map_children;


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

    struct ASTConditional : public ASTNode {
        std::unordered_map<ASTExpr *,ASTBlock> cases;
    };

    struct ASTFuncDecl : public ASTNode {
        std::string id;
        std::vector<std::string> params;
        ASTBlock body;
    };

    struct ASTVarDecl : public ASTNode {
        std::string id;
        std::optional<ASTExpr *> init;
    };

    
};

#endif