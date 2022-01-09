#include "Autom.h"

#include <filesystem>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <ShlObj.h>
#else
#include <glob.h>
#include <unistd.h>
#endif



using namespace autom;

AUTOM_NATIVE_FUNC(exists){
    auto val = objectToString(args[0].second);
    auto res = std::filesystem::exists(val.data());
    return toObject(res);
};

AUTOM_NATIVE_FUNC(abspath){
    auto val = objectToString(args[0].second);
    auto res = std::filesystem::absolute(std::filesystem::path(val.data()).lexically_normal()).string();
    return toObject(res);
};


AUTOM_NATIVE_FUNC(fs_glob){
    
    auto path = objectToString(args[0].second);
    
    std::vector<Object *> res;

    std::string str;
    
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFileA(std::filesystem::path(path.data()).string().c_str(),&findData);

    str = std::filesystem::path(path.data()).parent_path().append(findData.cFileName).string();
    
    res.push_back(toObject(str));

    while(FindNextFileA(h,&findData) == TRUE){
        str = std::filesystem::path(path.data()).parent_path().append(findData.cFileName).string();

        res.push_back(toObject(str));
    };

    FindClose(h);

#else
    
    glob_t g;
//    std::cout << path.data() << std::endl;
    
    int r = glob(path.data(),GLOB_MAGCHAR,NULL,&g);
//    std::cout << r << std::endl;
    
    if(r == 0){
        
        auto pathv = g.gl_pathv;
    
        auto pathc = g.gl_pathc;
        
        while(pathc > 0){
            
            std::string str(*pathv);
            res.push_back(toObject(str));
            
            pathc -= 1;
            ++pathv;
        }
        
    }
    
//    globfree(g);
    
#endif

    return toObject(res);
};

AUTOM_NATIVE_FUNC(mkdir){
    auto path = objectToString(args[0].second);
    
    std::filesystem::create_directories(path.data());
    
    return VOID_OBJECT;
}

AUTOM_NATIVE_FUNC(symlink){
    
    auto path_src = objectToString(args[0].second);
    
    auto path_dest  = objectToString(args[1].second);
    
    std::filesystem::create_symlink(path_src.data(),path_dest.data());
    
    return VOID_OBJECT;
}



AUTOM_EXT_INIT(){
    auto ext = new Extension;
    ext->name = "fs";
    ext->funcs.push_back({"fs_exists",exists,{{"path",StringObject}}});
    ext->funcs.push_back({"fs_abspath",abspath,{{"path",StringObject}}});
    ext->funcs.push_back({"fs_glob",fs_glob,{{"path",StringObject}}});
    ext->funcs.push_back({"fs_mkdir",mkdir,{{"path",StringObject}}});
    ext->funcs.push_back({"fs_symlink",symlink,{{"src",StringObject},{"dest",StringObject}}});
    return ext;
};

