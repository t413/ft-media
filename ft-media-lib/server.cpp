#include "server.h"
#include "utils.h"
#include "mediaplayer.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <udp-flaschen-taschen.h>


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
        options_.add_flag(  "-v,--verbose",   verbosity_, "Set the verbosity");
        options_.set_help_flag("--help", options_.get_help_ptr()->get_description());
        options_.add_option("-h,--host",      host_, "UDP host to transmit to");
        options_.add_option("-g,--geometry",  bind(&Server::parseGeometry, this, _1), "<W>x<H>[+<X>+<Y>[+<layer>]]");
        options_.add_option("-t,--minrepeat", minRepeatTime_, "minimum time to play a video, will repeat");
        options_.add_option("gifs,-f,--gifs", files_, "");
    }

    volatile bool* isRunningPtr = NULL;
    static void InterruptHandler(int) { if (isRunningPtr) *isRunningPtr = false; }

    bool Server::init() {
        const int ft_socket = OpenFlaschenTaschenSocket(host_.c_str());
        if (ft_socket < 0) {
            fprintf(stderr, "Couldn't open socket to FlaschenTaschen\n");
            return -1;
        }

        display_.reset(new UDPFlaschenTaschen(ft_socket, height_, width_));
        display_->SetOffset(offx_, offy_, offz_);

        MediaPlayer::Init();

        isRunningPtr = &isRunning_;
        signal(SIGTERM, InterruptHandler);
        signal(SIGINT, InterruptHandler);

        debug("got files: " + util::join(files_));
        int numberPlayed = 0;
        for (const string &video : files_) {
            try {
                MediaPlayer media(video, *this, display_);
                media.play();
                numberPlayed++;
            } catch (runtime_error e) {
                debug("error playing " + video + ": " + string(e.what()));
            }
            if (!isRunning_) {
                debug("Got interrupt. Exiting");
                break;
            }
        }
        display_->Clear();
        display_->Send();
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
