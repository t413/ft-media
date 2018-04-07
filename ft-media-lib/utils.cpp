#include "utils.h"
#include <iostream>
#include <sys/time.h>
#include <sstream>

namespace ftm {
    using namespace std;

    namespace util {

        double timeNow() {
            struct timeval tp;
            gettimeofday(&tp, NULL);
            return tp.tv_sec + tp.tv_usec;
        }

        int64_t timeMs() {
            return (int64_t) (util::timeNow() * 1000);
        }

        std::string join(const StringVec &l, std::string sep) {
            return join(StringLst(l.begin(), l.end()), sep);
        }

        std::string join(const StringLst &l, std::string sep) {
            std::ostringstream ret;
            StringLst::const_iterator it = l.begin();
            for (; it != l.end(); it++) {
                ret << *it;
                if (it != l.end()) ret << sep;
            }
            return ret.str();
        }

    }
}
