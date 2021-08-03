#include "ADT.h"

#include <iostream>
#include <tuple>

#ifndef AUTOM_DIAGNOSTIC_H
#define AUTOM_DIAGNOSTIC_H

namespace autom {

    struct FormatterTemplateBase {
        virtual void sub(std::ostream & out) = 0;
    };

    template<class T>
    struct FormatterTemplate : public FormatterBase {
        T val;
        FormatterTemplate(T val):val(val){

        };
        void sub(std::ostream &out) override {
            FormatterTemplate<T>::format(out,val);
        };
    };
    template<class T>
    FormatterTemplateBase * make_formatter(T && val){
        return new FormatterTemplate(val);
    };

    template<class ...Args>
    void formatmsg(autom::StrRef msg,Args && ...args){
        auto argstemp = std::make_tuple(args...);
        std::array<FormatterTemplateBase *,std::tuple_size<decltype(argstemp)>> {make_formatter(args)...};

    };

}

#endif