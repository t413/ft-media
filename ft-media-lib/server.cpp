#include "server.h"
#include "utils.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#include <udp-flaschen-taschen.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <ao/ao.h>
}


namespace ftm {
    using namespace std;
    using namespace std::placeholders;

    Server::Server() :
            options_("FT-Media Server"),
            host_("localhost"),
            height_(64),
            width_(64),
            offx_(0),
            offy_(0),
            offz_(3) {
        options_.add_flag("-v,--verbose", verbosity_, "Set the verbosity");
        options_.add_option("-g,--geometry", bind(&Server::parseGeometry, this, _1), "<W>x<H>[+<X>+<Y>[+<layer>]]");
        options_.add_option("gifs,-f,--gifs", files_, "");
    }

    volatile bool interrupt_received = false;
    static void InterruptHandler(int) { interrupt_received = true; }

    bool Server::init() {
        const int ft_socket = OpenFlaschenTaschenSocket(host_.c_str());
        if (ft_socket < 0) {
            fprintf(stderr, "Couldn't open socket to FlaschenTaschen\n");
            return -1;
        }

        display_.reset(new UDPFlaschenTaschen(ft_socket, height_, width_));
        display_->SetOffset(offx_, offy_, offz_);

        // Register all formats and codecs
        av_register_all();
        avformat_network_init();

        signal(SIGTERM, InterruptHandler);
        signal(SIGINT, InterruptHandler);

        debug("got files: " + util::join(files_));
        int numberPlayed = 0;
        for (const string &video : files_) {

            //bool playresult = PlayVideo(movie_file, display, audio_id, verbose, repeatTimeout);
            //if (playresult)
                numberPlayed++;

            if (interrupt_received) {
                debug("Got interrupt. Exiting");
                break;
            }
            display_->Clear();
            display_->Send();
        }
        return !numberPlayed;
    }

    void Server::debug(string msg) const {
        cout << msg << endl;
    }

    bool Server::parseGeometry(vector<string> opt) {
        return sscanf(opt.front().c_str(), "%dx%d%d%d%d", &width_, &height_, &offx_, &offy_, &offz_) > 2;
    }

    int Server::run(int argc, char *argv[]) {
        try {
            options_.parse(argc, argv);
        } catch(const CLI::ParseError &e) {
            return options_.exit(e);
        }

        cout << "hello world" << endl;
        cout << "verbosity is " << verbosity_ << endl;

        return init();
    }


}
