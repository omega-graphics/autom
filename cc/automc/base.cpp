#include "base.h"
#include <string>
#include <sstream>



namespace automc {
    namespace fs {
        Path::Path(const std::string &value) {
            parse(value);
        }

        void Path::parse(const std::string &value) {
            char tokBuffer[150];
            char *beginPtr,*endPtr;
            std::istringstream is(value);

            beginPtr = tokBuffer;
            endPtr = beginPtr;

            auto aheadChar = [&]() -> char{
                char c = is.get();
                is.seekg(-1,std::__1::ios_base::cur);
                return c;
            };

            auto pushToken = [&](){
                auto length = endPtr - beginPtr;
                toks.emplace_back(std::string(beginPtr,length));
                endPtr = beginPtr;
            };

        #define PUSH_CHAR(c) *endPtr = c;++endPtr;

            char c;
            while((c = is.get()) != -1){
                switch (c) {
                    case '/':
                    {
                        PUSH_CHAR(c)
                        pushToken();
                        break;
                    }
                    case '\0': {
                        return;
                    }
                    default: {
                        if(isalnum(c)){
                            PUSH_CHAR(c)
                            c = aheadChar();
                            if(!isalnum(c)){
                                pushToken();
                            };
                        };
                        break;
                    }
                }
            }
        }

        std::string Path::str() {
            std::string res;
            for(auto & tok : toks){
                res += tok;
            };
            return res;
        }

        DirectoryIterator::DirectoryIterator(const Path& path):path(path),hasEnded(false) {
        #if defined(UNIX_FS)
                dir = opendir(this->path.str().c_str());
        #endif
        }

        DirectoryIterator::Entry DirectoryIterator::get() {
            const dirent *dirent;
            std::string name;
            Entry::Type ty;
            if((dirent = readdir(dir)) != 0){
                name = dirent->d_name;

                if(dirent->d_type == DT_REG){
                    ty = Entry::RegularFile;
                }
                else if(dirent->d_type == DT_DIR){
                    ty = Entry::Directory;
                };
            }

            auto pos = telldir(dir) - 1l;
            if((readdir(dir) == 0)){
                hasEnded = true;
            }
            else {
                hasEnded = false;
            }
            seekdir(dir,pos);

            return {ty,path.str() + "/" + name};

        }



        bool DirectoryIterator::end() const {
            return hasEnded;
        }

        DirectoryIterator::~DirectoryIterator() {
            closedir(dir);
        }
    };
};