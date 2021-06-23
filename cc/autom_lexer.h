#include <stdio.h>
#include "toks.def"

#ifndef AUTOM_LEXER_H
#define AUTOM_LEXER_H


typedef struct __AutomLangTok {
    int type;
    char *content;
    size_t content_length;
} AutomLangTok;

typedef struct AutomLangLexer AutomLangLexer;

AutomLangLexer *autom_lexer_new();
void autom_lexer_set_input_stream(AutomLangLexer *,FILE *);
AutomLangTok * autom_lexer_next_tok(AutomLangLexer *);
void autom_lexer_free(AutomLangLexer *);


#endif