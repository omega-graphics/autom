#include "../engine/Lexer.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include "ADT.h"

int main(int argc,char *argv[]){

    autom::StrRef file(argv[1]);

    autom::Lexer lexer;
    std::ifstream in(file);
    lexer.setInputStream(&in);
    std::vector<autom::Tok> tokens;
    lexer.tokenize(&tokens);
    for(auto & t : tokens){
        std::cout << "{ Type:" << std::hex << t.type << std::dec << ",\n Str:" << std::quoted(t.str) << "}" << std::endl;
    };

    return 0;
};