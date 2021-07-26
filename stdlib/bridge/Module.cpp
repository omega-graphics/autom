#include "Autom.h"

using namespace autom;

AUTOM_NATIVE_FUNC(gn_bridge);
AUTOM_NATIVE_FUNC(cmake_bridge);

AUTOM_EXT_INIT(){
    auto ext = new Extension;
    ext->name = "bridge";
    ext->funcs = {
        {"gn",gn_bridge},
        {"cmake",cmake_bridge}
    };
    return ext;
};