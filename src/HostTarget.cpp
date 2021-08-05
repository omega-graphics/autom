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
}

