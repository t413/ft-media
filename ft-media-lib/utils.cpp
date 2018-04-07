#include "util.h"
#include <iostream>
#include <sys/time.h>

namespace ftm {
    using namespace std;

    namespace util {

        int64_t timeMs() {
            return (int64_t) (time() * 1000);
        }

        double time() {
            struct timeval tp;
            gettimeofday(&tp, NULL);
            return tp.tv_sec + tp.tv_usec;
        }
    }



}
