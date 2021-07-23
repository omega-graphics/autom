#include "ASTFactory.h"
#include "Lexer.h"
// #include "Tokens.def"

namespace autom {

    ASTFactory::ASTFactory(Lexer &lexer):lexer(lexer){

    };

    void ASTFactory::setTokenVector(std::vector<Tok> *tokStream){
        this->tokStream = tokStream;
    };

    Tok & ASTFactory::nextToken(){
        return tokStream->at(privTokIndex++);
    };

    Tok & ASTFactory::aheadToken(){
         return tokStream->at(privTokIndex + 1);
    };

    ASTNode *ASTFactory::buildDecl(Tok & first_tok,ASTScope *scope){
        ASTNode *node;
        if(first_tok.str == KW_IMPORT){
            auto decl = new ASTImportDecl();
            decl->type = IMPORT_DECL;
            first_tok = nextToken();
            if(first_tok.type != TOK_STRLITERAL) {
                decl->value = first_tok.str.substr(1,first_tok.str.size()-2);
            }
            else {
                return nullptr;
            }
            node = decl;
        };
        node->scope = scope;
        return node;
    };

    ASTNode *ASTFactory::buildExpr(Tok & first_tok,ASTScope *scope){
        ASTNode *expr;
        switch (first_tok.type) {
            case TOK_STRLITERAL : {
                auto _expr = new ASTLiteral();
                _expr->scope = scope;
                _expr->str = first_tok.str.substr(1,first_tok.str.size()-2);
                expr = _expr;
                break;
            }
        }
        return expr;
    };

    ASTNode *ASTFactory::nextStmt(){
       Tok &tok = nextToken();
       if(tok.type == TOK_EOF)
          return nullptr;
       if(tok.type == TOK_KW) { 
           return buildDecl(tok,GLOBAL_SCOPE);
       }
       else return buildExpr(tok,GLOBAL_SCOPE);
    };
    
}