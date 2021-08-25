#include "../engine/Lexer.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "ADT.h"

//#include <unistd.h>


int main(int argc,char *argv[]){

//    std::string path = getenv("PATH");
//    std::string progPath;
//    auto progFound = autom::locateProgram("cmake",path,progPath);
//    if(progFound){
//        std::cout << "Program Found:" << progPath << std::endl;
//    }
//    else {
//        std::cout << "Program not found" << std::endl;
//    }
    
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
