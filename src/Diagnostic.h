#include "ADT.h"

#include <iostream>
#include <tuple>
#include <sstream>

#ifndef AUTOM_DIAGNOSTIC_H
#define AUTOM_DIAGNOSTIC_H

namespace autom {

    struct FormatterTemplateBase {
        virtual void sub(std::ostream & out) = 0;
    };

    template<typename T>
    struct FormatterTemplate : public FormatterTemplateBase {
        T val;
        explicit FormatterTemplate(T && val):val(val){

        };

        void sub(std::ostream &out) override {
            Format<T>::format(out,val);
        };
    };

    template<typename T>
    struct Format;

    template<>
    struct Format<std::string> {
        static void format(std::ostream & out,std::string & val){
            out << val;
        };
    };

    template<>
    struct Format<StrRef> {
        static void format(std::ostream & out,autom::StrRef & val){
            out << val.data();
        };
    };


    template<typename T>
    FormatterTemplateBase * make_formatter(T val){
        return (FormatterTemplateBase *)new FormatterTemplate<T>(std::forward<T>(val));
    };

    void formatWithTemplate(StrRef & msg,std::ostringstream & out,FormatterTemplateBase **formatters,size_t formattersN);

    template<typename ...Args>
    std::string formatmsg(StrRef msg,Args && ...args){
        std::array<FormatterTemplateBase *,sizeof...(args)> formatters{make_formatter(std::forward<Args>(args))...};
        std::ostringstream out;
        formatWithTemplate(msg,out,formatters.data(),formatters.size());
        return out.str();
    };

}

#endif