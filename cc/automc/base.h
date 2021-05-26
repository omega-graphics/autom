#include <string>

#ifndef AUTOMC_BASE_H
#define AUTOMC_BASE_H

namespace automc {
    namespace fs {

        class Path {
        
        void parse(std::string & value);
            public:
            Path(const std::string & value);
        };

        class DirectoryIterator {

        };

    };

};

#endif