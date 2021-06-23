#include "autom_lexer.h"
#include "autom_util.h"
#include <stdio.h>
#include <ctype.h>

struct AutomLangLexer {
    FILE *_stream;
};

char autom_lexer_next_char(AutomLangLexer *lexer){
    return fgetc(lexer->_stream);
}

char autom_lexer_ahead_char(AutomLangLexer *lexer){
    char ahead_ch = fgetc(lexer->_stream);
    fseeko(lexer->_stream,-1,SEEK_CUR);
    return ahead_ch;
}





AutomLangTok * autom_lexer_next_tok(AutomLangLexer *lexer){
    
    char buffer[200];
    char * bufPtrBeg = buffer;
    char * bufPtrEnd = bufPtrBeg;

    #define PUSH_CHAR(c) *bufPtrEnd = c;\
    ++bufPtrEnd;
    
    #define PUSH_TOK(_type) \
    AutomLangTok t; \
    AutomLangTok *tok = autom_alloc_with_data(&t,sizeof(t));\
    tok->type = _type;\
    size_t buffer_len = bufPtrEnd - bufPtrBeg;\
    tok->content = autom_alloc_with_data(buffer,buffer_len);\
    tok->content_length = buffer_len;\
    return tok;

    char ch;
    while((ch = autom_lexer_next_char(lexer)) != -1){
        switch(ch){
            case ':': {
                PUSH_CHAR(ch)
                PUSH_TOK(TOK_COLON)
                break;
            }
            case '.': {
                PUSH_CHAR(ch)
                PUSH_TOK(TOK_DOT)
                break;
            }
            default : {
                if(isalnum(ch)){
                    PUSH_CHAR(ch)
                    ch = autom_lexer_ahead_char(lexer);
                    if(!isalnum(ch))
                    {
                        PUSH_TOK(TOK_ID)
                    }
                }
                else {
                    return NULL;
                }
                break;
            }
        }
    };
    PUSH_CHAR(ch)
    PUSH_TOK(TOK_EOF)
}

AutomLangLexer *autom_lexer_new(){
    AutomLangLexer s;
    return autom_alloc_with_data(&s,sizeof(s));
}

void autom_lexer_set_input_stream(AutomLangLexer *lexer,FILE *stream){
    lexer->_stream = stream;
}

void autom_lexer_free(AutomLangLexer *lexer){
    autom_free_ptr(lexer);
}