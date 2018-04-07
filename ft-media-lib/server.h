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

        int verbosity_ = 0; //0 normal, 1 extra, 2 cray
        bool isRunning_ = true;
        double minRepeatTime_ = 0;
    protected:
        CLI::App options_;

        std::string host_;
        int height_, width_, offx_, offy_, offz_;
        UDPFTPtr display_;
        std::vector<std::string> files_;

    };

}
