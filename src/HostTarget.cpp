//
// Created by Pedestria on 8/4/2021.
//

#include "Target.h"

#ifdef _WIN32
#include <Windows.h>
#else 
#include <sys/utsname.h>
#endif

namespace autom {




    TargetArch hostArch(){
        TargetArch arch;
#ifdef _WIN32
        
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        switch (systemInfo.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64 : {
                arch = TargetArch::x86_64;
                break;
            }
            case PROCESSOR_ARCHITECTURE_ARM : {
                arch = TargetArch::ARM;
                break;
            }
            case PROCESSOR_ARCHITECTURE_ARM64 : {
                arch = TargetArch::AARCH64;
                break;
            }
            case PROCESSOR_ARCHITECTURE_INTEL : {
                arch = TargetArch::x86;
                break;
            }
        }
       
    #else 
        utsname info;
        uname(&info);
        
        StrRef archStr {info.machine};

        if(archStr == "arm64"){
            arch = TargetArch::AARCH64;
        }
        else if(archStr == "arm"){
            arch = TargetArch::ARM;
        }
        else if(archStr == "x86"){
            arch = TargetArch::x86;
        }
        else if(archStr == "x86_64"){
            arch = TargetArch::x86_64;
        }
        
#endif
     return arch;
    }

    TargetOS hostOS(){
#ifdef _WIN32
        return TargetOS::Windows;
#elif defined(__APPLE__)
        return TargetOS::Darwin;
#endif
    }

    TargetPlatform hostPlatform(){
#ifdef _WIN32
        return TargetPlatform::Windows;
#elif defined(__APPLE__)
        return TargetPlatform::macOS;
#endif
    }

/// @name Os
/// @{

static const char Windows[] = "windows";
static const char Darwin[] = "darwin";
static const char Linux[] = "linux";

/// @}

/// @name Arch
/// @{

static const char x86[] = "x86";
static const char x86_64[] = "x86_64";
static const char aarch64[] = "aarch64";
static const char arm[] = "arm";

/// @}

/// @name Platforms
/// @{

static const char WindowsPlatform[] = "windows";
static const char macOS[] = "macos";
static const char LinuxPlatform[] = "linux";
static const char iOS[] = "ios";
static const char Android[] = "android";

/// @}

const char * TargetOSToString(TargetOS &os){
    switch (os) {
        case TargetOS::Windows: {
            return Windows;
            break;
        }
        case TargetOS::Darwin : {
            return Darwin;
            break;
        }
        case TargetOS::Linux : {
            return Linux;
            break;
        }
    }
}

const char * TargetArchToString(TargetArch & arch){
    switch (arch) {
        case TargetArch::x86: {
            return x86;
            break;
        }
        case TargetArch::x86_64 :{
            return x86_64;
            break;
        }
        case TargetArch::ARM : {
            return arm;
        }
        case TargetArch::AARCH64 : {
            return aarch64;
        }
    }
}

const char *TargetPlatformToString(TargetPlatform & platform){
    switch (platform) {
        case TargetPlatform::Windows : {
            return WindowsPlatform;
            break;
        }
        case TargetPlatform::macOS : {
            return macOS;
            break;
        }
        case TargetPlatform::Linux : {
            return LinuxPlatform;
            break;
        }
        case TargetPlatform::iOS : {
            return iOS;
            break;
        }
        case TargetPlatform::Android : {
            return Android;
            break;
        }
    }
}


}

