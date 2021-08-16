#include "../engine/Lexer.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "ADT.h"

#include <CommonCrypto/CommonDigest.h>

inline std::string generateObjectID(autom::StrRef name,autom::StrRef otherData){
   
    return out.str();
};

int main(int argc,char *argv[]){


    
    std::string src = "Autom";
    std::string type = "PBXNativeTarget";
    
    std::cout << "Source:" << src << type  << std::endl;
    std::cout << "ID:" << generateObjectID(src,type) << std::endl;
    
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