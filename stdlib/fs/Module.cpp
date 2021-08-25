#include "Autom.h"

#include <filesystem>




MODULE_EXPORT int testInt(){ return 50;};


using namespace autom;

AUTOM_NATIVE_FUNC(exists){
    auto val = objectToString(args[0].second);
    auto res = std::filesystem::exists(val.data());
    return toObject(res);
};

AUTOM_NATIVE_FUNC(abspath){
    auto val = objectToString(args[0].second);
    auto res = std::filesystem::absolute(val.data()).string();
    return toObject(res);
};


//
//class FileGlobber {
//
//};
//
//AUTOM_NATIVE_FUNC(glob){
//    auto val = objectToString(object->second);
//    auto res = std::filesystem::exists(val.data());
//    return toObject(res);
//};



AUTOM_EXT_INIT(){
    auto ext = new Extension;
    ext->name = "fs";
    ext->funcs.push_back({"fs_exists",exists,{{"path",StringObject}}});
    ext->funcs.push_back({"fs_abspath",abspath,{{"path",StringObject}}});
    return ext;
};

