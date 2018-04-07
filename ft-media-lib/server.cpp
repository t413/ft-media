#include "server.h"
#include <iostream>

namespace ftm {
    using namespace std;

    Server::Server() : options_("FT-Media Server") {

        //options_.add_option("-f,--file", filename, "A help string");
        options_.add_flag("-v,--verbose", verbosity_, "Set the verbosity");

    }

    int Server::run(int argc, char *argv[]) {
        CLI11_PARSE(options_, argc, argv);
        cout << "hello world" << endl;
        cout << "verbosity is " << verbosity_ << endl;
    }


}
