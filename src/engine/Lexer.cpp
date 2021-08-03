#include "Lexer.h"
#include "../ADT.h"
#include "engine/Tokens.def"
#include <cctype>

namespace autom {

    inline bool isKeyword(StrRef subject){
        return (subject == KW_IF ) || (subject == KW_ELIF) || (subject == KW_FUNC) || (subject == KW_VAR) || (subject == KW_IMPORT);
    };


    void Lexer::setInputStream(std::istream *is){
        this->is = is;
    };



    void Lexer::tokenize(std::vector<Tok> *tokenVector){
        char *bufSt = chBuf,*bufEnd = bufSt;

        #define PUSH_TOKEN(type)\
            auto t = type;\
            auto bufLen = bufEnd - bufSt;\
            auto str = autom::StrRef(chBuf,bufLen);\
            if(isKeyword(str)){\
                 t = TOK_KW;    \
            };\
            tokenVector->push_back(Tok {str,t});\
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
                case '=' : {
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_EQUALS)
                    break;
                }
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
                case '"' : {
                    PUSH_CHAR(c)
                    while((c = getChar()) != '"') {
                        PUSH_CHAR(c)
                    }
                    PUSH_CHAR(c)
                    PUSH_TOKEN(TOK_STRLITERAL)
                    break;
                }
                case '/' : {
                    char prevChar;
                    while(true) {
                        prevChar = c;
                        c = getChar();
                        
                        if(prevChar != '\\' && c == '/'){
                            break;
                        }

                        PUSH_CHAR(c)
                    }
                    // Last Char in Regex
                    PUSH_TOKEN(TOK_REGEXLITERAL)
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