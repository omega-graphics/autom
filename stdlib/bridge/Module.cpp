#include "Autom.h"

using namespace autom;

AUTOM_NATIVE_FUNC(gn_bridge);
AUTOM_NATIVE_FUNC(cmake_bridge);

AUTOM_EXT_INIT(){
    auto ext = new Extension;
    ext->name = "bridge";
    ext->funcs = {
        {"bridge_gn",gn_bridge},
        {"bridge_cmake",cmake_bridge}
    };
    return ext;
};
