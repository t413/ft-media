#pragma once
#include <string>
#include <list>
#include "addons/CLI11.hpp"

class UDPFlaschenTaschen;

namespace ftm {
    typedef std::shared_ptr<UDPFlaschenTaschen> UDPFTPtr;

    class Server {
    public:
        Server(); //sets up CLI options, defaults
        bool init(); //begins the app running

        int run(int argc, char *argv[]); //parses CLI from main(), calls init

        void debug(std::string) const;
        bool parseGeometry(std::vector<std::string>);

    protected:
        CLI::App options_;
        int verbosity_{1}; //0 none, 1 normal, 2 extra, 3 cray

        std::string host_;
        int height_, width_, offx_, offy_, offz_;
        UDPFTPtr display_;
        std::vector<std::string> files_;

    };

}
