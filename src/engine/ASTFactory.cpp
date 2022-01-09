#include "ASTFactory.h"
#include "Lexer.h"
#include "engine/AST.def"
#include "engine/Tokens.def"
#include "ExecEngine.h"
#include "Diagnostic.h"

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
                engine->printError("Expected A STR Literal");
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
        }
        else if(first_tok.str == KW_FUNC){
            auto *decl = new ASTFuncDecl();
            decl->type = FUNC_DECL;
            first_tok = nextToken();
            
            if(first_tok.type != TOK_ID){
                std::cout << "Expected an ID" << std::endl;
                return nullptr;
            }
            
            decl->id = first_tok.str;
            
            if(nextToken().type != TOK_LPAREN){
                std::cout << "Expected an LParen" << std::endl;
                return nullptr;
            }
            
            while((first_tok = nextToken()).type != TOK_RPAREN){
                
                decl->params.push_back(first_tok.str);

                first_tok = nextToken();
                
                
                if(first_tok.type == TOK_COMMA && aheadToken().type == TOK_RPAREN){
                    std::cout << "No Comma allowed before closing RParen" << std::endl;
                    return nullptr;
                }
                else if(first_tok.type == TOK_COMMA){
                    continue;
                }
                else if(first_tok.type == TOK_RPAREN){
                    break;
                }
                else {
                    std::cout << "Comma or RParen Expected" << std::endl;
                    return nullptr;
                }
            }
            
            first_tok = nextToken();
            
            decl->body.reset(buildBlock(first_tok,scope));
            
            node = decl;
            
        }
        else if(first_tok.str == KW_FOREACH){
            auto *decl = new ASTForeachDecl();
            decl->type = FOREACH_DECL;
            first_tok = nextToken();
            if(first_tok.type != TOK_ID){
                std::cout << "Expected an ID" << std::endl;
                return nullptr;
            }
            decl->loopVar = first_tok.str;
            first_tok = nextToken();
            if(first_tok.type != TOK_KW && first_tok.str != KW_IN){
                std::cout << "Expected Keyword `in` in this context" << std::endl;
                return nullptr;
            }
            first_tok = nextToken();
            auto expr = evalArgsExpr(first_tok,scope);
            if(!expr){
                return nullptr;
            }
            decl->list = expr;
            
            first_tok = nextToken();
            
            auto b = buildBlock(first_tok,scope);
            if(!b){
                return nullptr;
            }
            
            decl->body.reset(b);
            node = decl;
        }
        else if(first_tok.str == KW_IF){
            auto * decl = new ASTConditionalDecl();
            decl->type = COND_DECL;
            
            ASTConditionalDecl::CaseSpec spec;
            
            auto buildSpec = [&](){
                bool wrapParen;
                first_tok = nextToken();
                if(first_tok.type != TOK_LPAREN){
                    wrapParen = true;
                    first_tok = nextToken();
                }
                else {
                    wrapParen = false;
                }
                
                auto testCase = buildExpr(first_tok,scope);
                if(!testCase){
                    return;
                }
                
                if(wrapParen){
                    if(nextToken().type != TOK_RPAREN){
                        engine->printError("Expected RParen in this context.");
                        return;
                    }
                }
                first_tok = nextToken();
                spec.testCondition = true;
                spec.condition = testCase;
                spec.block.reset(buildBlock(first_tok,scope));
            };
            
            buildSpec();
            decl->cases.push_back(std::move(spec));
            first_tok = aheadToken();
            
            while(first_tok.type == TOK_KW){
                spec = ASTConditionalDecl::CaseSpec {false,nullptr,nullptr};
                
                if(first_tok.str == KW_ELIF){
                    incToNextToken();
                    buildSpec();
                }
                else if(first_tok.str == KW_ELSE) {
                    incToNextToken();
                    first_tok = nextToken();
                    spec.condition = nullptr;
                    spec.testCondition = false;
                    spec.block.reset(buildBlock(first_tok,scope));
                }
                else {
                    break;
                }
                decl->cases.push_back(std::move(spec));
                first_tok = aheadToken();
            }
            node = decl;
        }
        else if(first_tok.str == KW_ELIF || first_tok.str == KW_ELSE){
            engine->printError("Cannot use keywords `elif` or `else` in this context.");
            return nullptr;
        }
        else if(first_tok.str == KW_RETURN){
            auto *decl = new ASTExpr();
            decl->type = RETURN_DECL;
            decl->rhs = buildExpr(nextToken(),scope);
            node = decl;
        }
        else {
            return nullptr;
        }
        
        ASTScopeAddReference(scope);
        node->scope = scope;
        return node;
    };

    ASTBlock * ASTFactory::buildBlock(Tok & first_tok,ASTScope *scope){
        if(first_tok.type != TOK_LBRACE){
            std::cout << "ERROR: Expected LBrace" << std::endl;
            return nullptr;
        }
        
        auto block = new ASTBlock();
        
        ASTScopeAddReference(scope);
        auto block_scope = ASTScopeCreate("__BLOCK_SCOPE__",scope);
        block->scope = block_scope;
        
        while((first_tok = nextToken()).type != TOK_RBRACE){
            ASTNode *_node;
            
            if(first_tok.type == TOK_KW){
                _node = buildDecl(first_tok,block_scope);
            }
            else {
                _node = buildExpr(first_tok,block_scope);
            }
            
            if(!_node){
                return nullptr;
            }
            
            block->body.push_back(_node);
            
        }
        
        return block;
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
                
            // Boolean Literal
            case TOK_BOOLLITERAL : {
                auto _expr = new ASTLiteral();
                _expr->type = EXPR_LITERAL;
                _expr->boolean = first_tok.str == BOOL_TRUE? true : first_tok.str == BOOL_FALSE? false : false;
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
        ASTScopeAddReference(scope);
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
            ASTScopeAddReference(scope);
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
            case TOK_PLUS :
            case TOK_PLUSEQUALS :
            case TOK_EQUALS_COND :
            case TOK_EQUALS_NOT_COND :
                binary_expr();
                break;
            default : {
//                engine->printError(formatmsg("Unknown Token @0.",first_tok.str));
                break;
            }
        }
        ASTScopeAddReference(scope);
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
                engine->printError("Expected RParen");
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
