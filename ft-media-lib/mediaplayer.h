#pragma once
#include <string>
#include <vector>
#include <list>

struct AVFrame;
struct AVFormatContext;
class UDPFlaschenTaschen;

namespace ftm {
    class Server;
    typedef std::shared_ptr<UDPFlaschenTaschen> UDPFTPtr;


    class MediaPlayer {
    public:
        MediaPlayer(std::string name, Server&ctx, UDPFTPtr display);
        ~MediaPlayer();
        static void Init();

        void play();

    protected:
        void showFrame(AVFrame *pFrame);

    protected:
        std::string fname_;
        Server& ctx_;
        UDPFTPtr display_;
        AVFormatContext* avCtx_;

    };

}
