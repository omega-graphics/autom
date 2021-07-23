#include "Lexer.h"
#include "../ADT.h"
#include <cctype>

namespace autom {

    inline bool isKeyword(StrRef subject){
        return (subject == KW_IF ) || (subject == KW_ELIF) || (subject == KW_FUNC) || (subject == KW_VAR);
    };


    void Lexer::setInputStream(std::istream *is){
        this->is = is;
    };



    void Lexer::tokenize(std::vector<Tok> *tokenVector){
        char *bufSt = chBuf,*bufEnd = bufSt;

        #define PUSH_TOKEN(type)\
            auto bufLen = bufEnd - bufSt;\
            tokenVector->push_back(Tok {std::string(chBuf,bufLen),type});\
            bufEnd = bufSt;

        #define PUSH_CHAR(c)\
            *bufEnd = c;\
            ++bufEnd;

        auto getChar = [&](){
            return char(is->get()); 
        };

        auto aheadChar = [&](){
            char c = is->get();
            is->seekg(-1,std::ios::cur);
            return c;
        };


        char c;
        while((c = getChar()) != -1){
            switch(c){
                case ':' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_COLON)
                    break;
                }
                case ',' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_COMMA)
                    break;
                }
                case '(' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_LPAREN)
                    break;
                }
                case ')' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_RPAREN)
                    break;
                }
                case '[' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_LBRACKET)
                    break;
                }
                case ']' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_RBRACKET)
                    break;
                }
                case '{' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_LBRACE)
                    break;
                }
                case '}' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_RBRACE)
                    break;
                }
                case '.' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_DOT)
                    break;
                }
                default: {
                    if(std::isalnum(c)){
                        PUSH_CHAR(c)
                        c = aheadChar();
                        if(!std::isalnum(c)){
                            PUSH_TOKEN(TOK_ID)
                            break;
                        };
                    };
                    break;
                }
            };
        };
        PUSH_CHAR(c)
        PUSH_TOKEN(TOK_EOF)


    };

    void Lexer::finish(){
        is = nullptr;
    };
}