#include "ASTFactory.h"
#include "Lexer.h"
#include "engine/AST.def"
#include "engine/Tokens.def"

#include "ADT.h"

#include <iostream>

namespace autom {

    ASTFactory::ASTFactory(Lexer &lexer):lexer(lexer),privTokIndex(0),tokStream(nullptr){

    };

    void ASTFactory::setTokenVector(std::vector<Tok> *_tokStream){
        this->tokStream = _tokStream;
    };

    Tok & ASTFactory::nextToken(){
        Tok & first = tokStream->at(++privTokIndex);
//        while(first.type == TOK_LINECOMMENT){
//            first = tokStream->at(++privTokIndex);
//        }
        return first;
    };

    Tok & ASTFactory::aheadToken(){
         return tokStream->at(privTokIndex + 1);
    };

    Tok & ASTFactory::currentToken(){
         return tokStream->at(privTokIndex);
    };

    void ASTFactory::incToNextToken(){
        ++privTokIndex;
    };

    ASTNode *ASTFactory::buildDecl(Tok & first_tok,ASTScope *scope){
        ASTNode *node;
        if(first_tok.str == KW_IMPORT){
            auto decl = new ASTImportDecl();
            decl->isInterface = true;
            decl->type = IMPORT_DECL;
            first_tok = nextToken();
            if(first_tok.type == TOK_STRLITERAL) {
                decl->value = first_tok.str.substr(1,first_tok.str.size()-2);
            }
            else {
                /// Error!
                std::cout << "Expected A STR Literal" << std::endl;
                return nullptr;
            }
            node = decl;
        }
        else if(first_tok.str == KW_LOAD){
            auto decl = new ASTImportDecl();
            decl->isInterface = false;
            decl->type = IMPORT_DECL;
            first_tok = nextToken();
            if(first_tok.type == TOK_STRLITERAL) {
                decl->value = first_tok.str.substr(1,first_tok.str.size()-2);
            }
            else {
                /// Error!
                std::cout << "Expected A STR Literal" << std::endl;
                return nullptr;
            }
            node = decl;
        }
        else if(first_tok.str == KW_VAR){
            auto decl = new ASTVarDecl();
            decl->type = VAR_DECL;
            first_tok = nextToken();
            if(first_tok.type != TOK_ID){
                /// Error!
                std::cout << "Expected An ID" << std::endl;
                return nullptr;
            }
            decl->id = first_tok.str;

            auto & t = aheadToken();
            if(t.type == TOK_EQUALS){
                incToNextToken();
                first_tok = nextToken();
                auto expr = buildExpr(first_tok,scope);
                if(!expr){
                    return nullptr;
                };
                decl->init = (ASTExpr *)expr;
            };

            node = decl;
        };
        node->scope = scope;
        return node;
    };

    ASTExpr *ASTFactory::evalObjExpr(Tok & first_tok,ASTScope *scope){
        ASTExpr *expr;
        switch (first_tok.type) {
                
            // Identifier
            case TOK_ID : {
                auto _expr = new ASTExpr();
                _expr->type = EXPR_ID;
                _expr->id = first_tok.str;
                expr = _expr;
                break;
            }
                
            // String Literal
            case TOK_STRLITERAL : {
                auto _expr = new ASTLiteral();
                _expr->type = EXPR_LITERAL;
                _expr->str = first_tok.str.substr(1,first_tok.str.size()-2);
                expr = _expr;
                break;
            }
                
            // Array Expr
            case TOK_LBRACKET : {
                auto _expr = new ASTExpr();
                _expr->type = EXPR_ARRAY;
                first_tok = nextToken();
                while(first_tok.type != TOK_RBRACKET){
                    auto e = buildExpr(first_tok,scope);
                    _expr->children.push_back(e);
                    first_tok = nextToken();

                    if(first_tok.type == TOK_RBRACKET){
                        break;
                    }
                    else if(first_tok.type != TOK_COMMA){
                        std::cout << "ERROR: Comma or RBracket Expected" << std::endl;
                        return nullptr;
                    }
                    
                    first_tok = nextToken();
                }
                expr = _expr;
                break;
            }
            default : {
                return nullptr;
                break;
            }
        }
        expr->scope = scope;
        return expr;
    };

    ASTExpr *ASTFactory::evalArgsExpr(Tok & first_tok,ASTScope *scope){
        auto obj = evalObjExpr(first_tok,scope);
        if(!obj){
            return nullptr;
        };
        ASTExpr *expr = obj;
        first_tok = aheadToken();
        // std::cout << "Tok T:" << "0x" << std::hex << first_tok.type << std::dec << "Tok C:" << first_tok.str << std::endl;
        bool stop = false;
        while(!stop) {
            switch (first_tok.type) {
                case TOK_LPAREN : {
                    incToNextToken();
                    ASTExpr *_expr= new ASTExpr();
                    _expr->type = EXPR_IVKE;
                    _expr->lhs = expr;

                    first_tok = nextToken();

                    // std::cout << "Tok T:" << "0x" << std::hex << first_tok.type << std::dec << "Tok C:" << first_tok.str << std::endl;
                    
                    while(first_tok.type != TOK_RPAREN){
                        auto paramId = first_tok.str;
                        // std::cout << "Tok T:" << "0x" << std::hex << first_tok.type << std::dec << "Tok C:" << first_tok.str << std::endl;
                        first_tok = nextToken();
                        if(first_tok.type != TOK_COLON){
                            /// Expected Colon
                            std::cout << "Expected COLON" << std::endl;
                            return nullptr;
                        };
                        first_tok = nextToken();
                        auto e = buildExpr(first_tok,scope);
                        if(!e){
                            return nullptr;
                        };
                        _expr->func_args.insert(std::make_pair(paramId,e));
                        first_tok = nextToken();
                        if(first_tok.type != TOK_COMMA && aheadToken().type == TOK_RPAREN){
                            incToNextToken();
                            break;
                        }
                        else if(first_tok.type == TOK_COMMA){
                            first_tok = nextToken();
                        };
                    };
                    expr = _expr;
                    break;
                }
                case TOK_DOT : {
                    incToNextToken();
                    ASTExpr *_expr= new ASTExpr();
                    _expr->type = EXPR_MEMBER;
                    _expr->lhs = expr;

                    first_tok = nextToken();

                    if(first_tok.type != TOK_ID){
                        /// Expected ID
                        std::cout << "Expected ID" << std::endl;
                    }
                    _expr->id = first_tok.str;
                    expr = _expr;
                    break;
                }
                default : { 
                    stop = true;
                    break;
                }
            }
            first_tok = aheadToken();
            expr->scope = scope;
        }
    
        return expr;
    };
    /// @note FIRST TOK IN THIS FUNCTION IS THE AHEAD TOKEN!!!!
    ASTExpr *ASTFactory::evalOpExpr(Tok & first_tok,ASTExpr *lhs,ASTScope *scope){
        
        
        
        if(!lhs){
            return nullptr;
        };
        ASTExpr *expr = lhs;
        
        auto binary_expr = [&]{
        
            incToNextToken();
            auto *_expr = new ASTExpr();
            _expr->type = EXPR_BINARY;
            _expr->lhs = lhs;
            _expr->operand = first_tok.str;
//            std::cout << "Operand: " << first_tok.str << std::endl;
            first_tok = nextToken();
            auto rhs = buildExpr(first_tok,scope);
            if(!rhs){
                return nullptr;
            };
            _expr->rhs = rhs;
            expr = _expr;
        };
        
        
        switch (first_tok.type) {
            case TOK_EQUALS : {
                incToNextToken();
                auto *_expr = new ASTExpr();
                _expr->type = EXPR_ASSIGN;
                _expr->lhs = lhs;
                first_tok = nextToken();
                auto rhs = buildExpr(first_tok,scope);
                if(!rhs){
                    return nullptr;
                };
                _expr->rhs = rhs;
                expr = _expr;
                break;
            }
            case TOK_PLUSEQUALS :
            case TOK_EQUALS_COND :
            case TOK_EQUALS_NOT_COND :
                binary_expr();
                break;
            default : {
                break;
            }
        }
        expr->scope = scope;
        return expr;
        
        
    };

    ASTExpr *ASTFactory::buildExpr(Tok & first_tok,ASTScope *scope){
        ASTExpr *expr;
        
        if(first_tok.type == TOK_LPAREN){
            first_tok = nextToken();
            auto ex = buildExpr(first_tok,scope);
            first_tok = nextToken();
            if(first_tok.type != TOK_RPAREN){
                /// Expected RParen!
                std::cout << "Expected RParen" << std::endl;
                return nullptr;
            };
            return ex;
        };

        expr = evalArgsExpr(first_tok,scope);
        first_tok = aheadToken();
        expr = evalOpExpr(first_tok,expr,scope);
        return expr;
    };

    ASTNode *ASTFactory::nextStmt(){
        // std::cout << "Next Stmt" << std::endl;
        Tok *tok;

        if(privTokIndex == 0){
            tok = &tokStream->at(0);
        }
        else {
            tok = &nextToken();
        }

       if(tok->type == TOK_EOF) {
          return nullptr;
       }

       if(tok->type == TOK_KW) {
           return buildDecl(*tok,GLOBAL_SCOPE);
       }
       else return buildExpr(*tok,GLOBAL_SCOPE);
    };
    
}
