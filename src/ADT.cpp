#include "ADT.h"

#include <iostream>

namespace autom {


    std::ostream & operator<<(std::ostream & os,StrRef & strRef){
        return os.write(strRef.data(),strRef.size());
    }

    std::ostream & operator<<(std::ostream & os,const StrRef & strRef){
        return os.write(strRef.data(),strRef.size());
    };

}