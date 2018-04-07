#pragma once
#include <string>
#include "addons/CLI11.hpp"

namespace ftm {

    class Server {
    public:
        Server();
        int run(int argc, char *argv[]);


    protected:
        CLI::App options_;
        int verbosity_{1}; //0 none, 1 normal, 2 extra, 3 cray

    };

}
