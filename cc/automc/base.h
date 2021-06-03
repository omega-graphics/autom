#include <string>
#include <vector>

#ifdef _WIN32
#define WINDOWS_FS 1
#else
#define UNIX_FS 1
#include <unistd.h>
#include <dirent.h>
#endif

#ifndef AUTOMC_BASE_H
#define AUTOMC_BASE_H

namespace automc {
    namespace fs {

        class Path {

            std::vector<std::string> toks;

            void parse(const std::string & value);
            public:
            std::string str();
            std::wstring wstr();
            Path(const std::string &value);
        };

        class DirectoryIterator {
            Path path;
#ifdef UNIX_FS
            DIR *dir;
#endif
        public:
            struct Entry {
                typedef enum : int{
                    RegularFile,
                    Directory
                } Type;
                Type type;
                Path path;
            };
            bool hasEnded;
            DirectoryIterator(const Path& path);
            Entry get();
            bool end() const;
            ~DirectoryIterator();
        };

    };

};

#endif