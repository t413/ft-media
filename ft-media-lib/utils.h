#pragma once
#include <string>
#include <vector>
#include <list>

namespace ftm {
    typedef std::vector<std::string> StringVec;
    typedef std::list<std::string> StringLst;
    namespace util {

        int64_t timeMs();
        double timeNow();

        std::string join(const StringLst&, std::string sep = ",");
        std::string join(const StringVec&, std::string sep = ",");

    }
}
