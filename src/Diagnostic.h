#include "ADT.h"

#include <iostream>
#include <tuple>
#include <sstream>

#ifndef AUTOM_DIAGNOSTIC_H
#define AUTOM_DIAGNOSTIC_H

namespace autom {

    template<typename T>
    struct Format;

    struct FormatterTemplateBase {
        virtual void sub(std::ostream & out) = 0;
    };

    template<class T>
    using has_format_provider = std::is_function<decltype(Format<T>::format)>;

    template<typename T,typename F = Format<T>>
    struct FormatterTemplate : public FormatterTemplateBase {
        T val;
        explicit FormatterTemplate(T && val):val(val){

        };

        void sub(std::ostream &out) override {
            F::format(out,val);
        };
    };

    

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

    template<>
    struct Format<const char *> {
        static void format(std::ostream & out,const char *& val){
            out << val;
        };
    };

    template<>
    struct Format<char> {
        static void format(std::ostream & out,char & val){
            out << val;
        };
    };

    template<>
    struct Format<int> {
        static void format(std::ostream & out,int & val){
            out << val;
        };
    };




    template<typename T,std::enable_if_t<has_format_provider<T>::value,int> = 0>
    FormatterTemplate<T> * make_formatter(T val){
        return new FormatterTemplate<T>(std::forward<T>(val));
    };

    void formatWithTemplate(StrRef & msg,std::ostringstream & out,FormatterTemplateBase **formatters,size_t formattersN);

    template<typename ...Formatters>
    struct FormatResult {
        std::string res;
        inline operator std::string(){
            return res;
        };
    };

    template<typename ...Args>
    auto formatmsg(StrRef msg,Args && ...args){
        std::array<FormatterTemplateBase *,sizeof...(args)> formatters{(FormatterTemplateBase *)make_formatter(std::forward<Args>(args))...};
        std::ostringstream out;
        formatWithTemplate(msg,out,formatters.data(),formatters.size());
        return FormatResult<decltype(std::make_tuple(make_formatter(std::forward<Args>(args))...))> {out.str()};
    };

}

#endif