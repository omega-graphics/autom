#include "Tokens.def"
#include <istream>
#include <vector>


#ifndef AUTOM_ENGINE_LEXER_H
#define  AUTOM_ENGINE_LEXER_H

namespace autom {
    /**
     A Token 
    */
    struct Tok {
        std::string str;
        TokType type;
    };

    class Lexer final {
        std::istream *is;

        char chBuf[TOKEN_MAX_LEN];

    public:

        void setInputStream(std::istream *is);

        void tokenize(std::vector<Tok> *tokenVector);
        
        void finish();
    };
}

#endif