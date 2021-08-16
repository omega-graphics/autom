#include "ADT.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include <filesystem>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#endif

namespace autom {
    
    typedef unsigned char HashByte;
    
#ifdef __APPLE__
    
    SHA256Hash::SHA256Hash():data(new CC_SHA256_CTX){
        CC_SHA256_Init((CC_SHA256_CTX *)data);
    }
    
    void SHA256Hash::addData(void * data,size_t dataSize){
        auto * ctxt = (CC_SHA256_CTX *)data;
        CC_SHA256_Update(ctxt,data,dataSize);
    };
    
    HashByte * SHA256Hash::getResult(){
        auto * hash = new HashByte[CC_SHA256_DIGEST_LENGTH];
        auto * ctxt = (CC_SHA256_CTX *)data;
        
        CC_SHA256_Final(hash,ctxt);
        return hash;
    }
    
    void SHA256Hash::getResultAsHex(std::string & out){
        
        auto *hash = getResult();
        
        std::ostringstream o(out);
        o << std::hex << std::uppercase;
        for(unsigned i = 0;i < CC_SHA256_DIGEST_LENGTH;i++){
            o << std::setw(2) << std::setfill('0') << int(hash[i]);
        }
    }
    
    SHA256Hash::~SHA256Hash(){
        delete (CC_SHA256_CTX *)data;
    }
    
#endif


    std::ostream & operator<<(std::ostream & os,StrRef & strRef){
        return os.write(strRef.data(),strRef.size());
    }

    std::ostream & operator<<(std::ostream & os,const StrRef & strRef){
        return os.write(strRef.data(),strRef.size());
    };

bool locateProgram(std::string prog,std::string path,char * out){
    
    std::istringstream in(path);
    std::string parentPath;

#ifdef _WIN32
    while(!in.eof()){
        std::getline(in,parentPath,':');
#else

    while(!in.eof()){
        std::getline(in,parentPath,':');

#endif
        
        auto p = std::filesystem::path(parentPath).append(prog);
        if(std::filesystem::is_symlink(p)){
            
            
        }
    }
    
};



}
