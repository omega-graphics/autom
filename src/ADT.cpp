#include "ADT.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include <filesystem>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#elif defined(_WIN32)
#include <Windows.h>
#include <bcrypt.h>

#pragma comment(lib,"bcrypt.lib")

#endif

namespace autom {
    
    typedef unsigned char HashByte;
    
#ifdef __APPLE__
    
    SHA256Hash::SHA256Hash():data(new CC_SHA256_CTX){
        CC_SHA256_Init((CC_SHA256_CTX *)data);
    }
    
    void SHA256Hash::addData(void * data,size_t dataSize){
        auto * ctxt = (CC_SHA256_CTX *)this->data;
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
#elif defined(_WIN32)

    SHA256Hash::SHA256Hash():data(new BCRYPT_HASH_HANDLE) {
        BCRYPT_ALG_HANDLE algHandle;
        BCryptOpenAlgorithmProvider(&algHandle,BCRYPT_SHA256_ALGORITHM,NULL,NULL);

        BCryptCreateHash(algHandle,(BCRYPT_HASH_HANDLE *)data,NULL,NULL,NULL,NULL,NULL);
        BCryptCloseAlgorithmProvider(algHandle,NULL);
    }

    void SHA256Hash::addData(void *data, size_t dataSize) {
        auto hashHandle = (BCRYPT_HASH_HANDLE *)this->data;
        BCryptHashData(*hashHandle,(PUCHAR)data,dataSize,NULL);
    }

    unsigned char *SHA256Hash::getResult() {
        PUCHAR hash = new UCHAR[32];
        auto hashHandle = (BCRYPT_HASH_HANDLE *)this->data;
        BCryptFinishHash(*hashHandle,hash,32,NULL);
        return hash;
    }

    void SHA256Hash::getResultAsHex(std::string &out) {
        auto *hash = getResult();

        std::ostringstream o(out);
        o << std::hex << std::uppercase;
        for(unsigned i = 0;i < 32;i++){
            o << std::setw(2) << std::setfill('0') << int(hash[i]);
        }
    }

    SHA256Hash::~SHA256Hash() {
        BCryptDestroyHash(*((BCRYPT_HASH_HANDLE *)data));
        delete (BCRYPT_HASH_HANDLE *)data;
    }
#endif


    std::ostream & operator<<(std::ostream & os,StrRef & strRef){
        return os.write(strRef.data(),strRef.size());
    }

    std::ostream & operator<<(std::ostream & os,const StrRef & strRef){
        return os.write(strRef.data(),strRef.size());
    };

bool locateProgram(autom::StrRef prog,std::string path,std::string & out){
    
    std::istringstream in(path);
    std::string parentPath;

#ifdef _WIN32
    while(!in.eof()){
        std::getline(in,parentPath,';');
#else

    while(!in.eof()){
        std::getline(in,parentPath,':');

#endif
        
        auto p = std::filesystem::path(parentPath).append(prog.data());
        if(std::filesystem::is_symlink(p)){
            out = std::filesystem::read_symlink(p).string();
            return true;
        }
        else {
#ifdef _WIN32
            p = p.replace_extension("exe");
            if(std::filesystem::exists(p)){
                out = p.string();
                return true;
            }
#else
            if(std::filesystem::exists(p)){
                out = p.string();
                return true;
            }
#endif
        }
    }
    return false;
    
};



}
