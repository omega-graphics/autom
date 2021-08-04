//
// Created by Pedestria on 8/4/2021.
//

#include "Target.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace autom {
    TargetArch hostArch(){
#ifdef _WIN32
        TargetArch arch;
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
        return arch;
#endif
    }

    TargetOS hostOS(){
#ifdef _WIN32
        return TargetOS::Windows;
#endif
    }

    TargetPlatform hostPlatform(){
#ifdef _WIN32
        return TargetPlatform::Windows;
#endif
    }
}

