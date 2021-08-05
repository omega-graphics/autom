#include "Diagnostic.h"
#include <sstream>
#include <cstdlib>
#include <cassert>

namespace autom {

    class TemplateLexer {
        char buffer[300];
        std::istringstream in;
        unsigned c_count,msg_len;
    public:
        bool finished() const{
            return c_count == msg_len;
        };
        explicit TemplateLexer(autom::StrRef & temp):in(temp),buffer(),c_count(0),msg_len(temp.size()){};
        struct Tok {
            bool isPlaceholder = false;
            int placeholderN = -1;
            std::string data;
        };
        Tok nextTok(){
            char *bufferSt = buffer,*bufferEnd = bufferSt;
            auto getChar = [&](){
                ++c_count;
                return (char)in.get();
            };
            auto aheadChar = [&](){
                char c = in.get();
                in.seekg(-1,std::ios::cur);
                return c;
            };
            char c;
            while((c = getChar()) != -1){
                switch (c) {
                    case '@': {
                        auto tmpc = c;
                        c = getChar();

                        if(std::isdigit(c)){
                            return {true,int(c - 48),""};
                        }
                        else {
                            *bufferEnd = tmpc;
                            ++bufferEnd;
                            *bufferEnd = c;
                            ++bufferEnd;
                        }
                        break;
                    }
                    default : {
                        *bufferEnd = c;
                        ++bufferEnd;
                        if(aheadChar() == '@'){
                            return {false,-1,{buffer,(size_t)(bufferEnd - bufferSt)}};
                            break;
                        }
                    }
                }
            }
        };
    };

    void formatWithTemplate(autom::StrRef & msg,std::ostringstream & out,FormatterTemplateBase **formatters,size_t formattersN){
        ArrayRef<FormatterTemplateBase *> f {formatters,formatters + formattersN};
        TemplateLexer lexer {msg};
        while(!lexer.finished()){
            TemplateLexer::Tok t = lexer.nextTok();
            if(t.isPlaceholder){
                assert(t.placeholderN < formattersN && "Sub number can not exceed template count");
                f.begin()[t.placeholderN]->sub(out);
            }
            else {
                out << t.data;
            }
        };
    };


}