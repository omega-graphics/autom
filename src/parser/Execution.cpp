#include "Execution.h"
#include "../Parser.h"

#include <filesystem>
#include <fstream>

namespace autom::eval {

    struct Object {
        typedef enum : int {
            Target, // data = Target *
            String, // data = StrData * 
            Array, // data = ArrayData *
            Boolean // data = BoolData *
        } Ty;
        Ty type;
        void *data;

        struct BoolData {
            bool data;
        };

        struct StrData {
            std::string data;
        };

        struct ArrayData {
            std::vector<Object *> data;
        };

    };

    Eval::Eval(TargetConsumer &targetConsumer,Parser *parser):parser(parser),targetConsumer(targetConsumer){

    };

    Object * Eval::evalExpr(ASTExpr *node){
        switch(node->type){
            case EXPR_LITERAL : {
                auto literal = (ASTLiteral *)node;
                if(literal->isString()){
                    return new Object {Object::String,new Object::StrData {literal->str.value()}};
                }
                else if(literal->isBoolean()){
                    return new Object {Object::Boolean,new Object::BoolData {literal->boolean.value()}};
                };
                break;
            }
            case EXPR_ID : {
                return referVarWithScope(node->scope,node->id);
                break;
            }
            case EXPR_ARRAY : {
                std::vector<Object *> objData;
                for(auto obj : node->children){
                    objData.push_back(evalExpr(obj));
                };
                return new Object {Object::Array,new Object::ArrayData {objData}};
                break;
            }
        };
        return nullptr;
    };

    bool Eval::evalStmt(ASTNode *node){
        if(node->type & EXPR){
            evalExpr((ASTExpr *)node);
        }
        else {
            switch (node->type) {
                case IMPORT_DECL : {
                   auto *decl = (ASTImportDecl *)node;
                   if(std::filesystem::exists(decl->value)){
                       std::ifstream in(decl->value);
                       parser->parseAndEvaluate(&in);
                   }
                   else {
                       /// Throw Error!
                        return false;
                   };
                   break;
                }
                case FUNC_DECL : {
                    funcs.push_back((ASTFuncDecl *)node);
                    break;
                }
                case VAR_DECL : {
                    auto *decl = (ASTVarDecl *)node;
                    Object *obj;
                    if(decl->init.has_value()){
                        obj = evalExpr(decl->init.value());
                    }
                    else {
                        obj = nullptr;
                    };
                    vars[node->scope].body.insert(std::make_pair(decl->id,obj));
                    break;
                }
            }
        };
        return true;
    };
    
}