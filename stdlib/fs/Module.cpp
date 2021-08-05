#include "Autom.h"

#include <filesystem>

using namespace autom;

AUTOM_NATIVE_FUNC(exists){
    auto val = objectToString(object->second);
    auto res = std::filesystem::exists(val.data());
    return toObject(res);
};

AUTOM_NATIVE_FUNC(abspath){
    auto val = objectToString(object->second);
    auto res = std::filesystem::absolute(val.data()).string();
    return toObject(res);
};


AUTOM_EXT_INIT(){
    auto ext = new Extension;
    ext->name = "fs";
    ext->funcs.push_back({"exists",exists});
    ext->funcs.push_back({"abspath",abspath});   
    return ext;
};

