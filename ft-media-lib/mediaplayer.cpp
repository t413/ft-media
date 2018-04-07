#include "mediaplayer.h"
#include "server.h"
#include "utils.h"
#include <iostream>
#include <udp-flaschen-taschen.h>
#include <assert.h>
#include <zconf.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <ao/ao.h>
}

namespace ftm {
    using namespace std;

    int getStreamType(AVFormatContext* ctx, enum AVMediaType type) {
        for (unsigned int i=0; i < ctx->nb_streams; ++i)
            if (ctx->streams[i]->codecpar->codec_type == type) return i;
        return -1;
    }

    void checkAlloc(void* data) {
        if (!data) cerr << "Can't allocate" << endl;
        assert(data);
    }

    struct MediaCtx {
        int index_ = -1;
        AVFormatContext* ctx_ = NULL;
        AVCodec* decoder_ = NULL;
        AVCodecContext* codecContext_ = NULL;
        AVFrame* frame_ = NULL;
        vector<uint8_t> buffer_;

        MediaCtx(AVFormatContext* ctx, enum AVMediaType type) {
            index_ = getStreamType(ctx, type);
            if (index_ < 0) throw runtime_error("finding stream error");

            const AVCodecParameters& params = * ctx->streams[index_]->codecpar;
            decoder_ = avcodec_find_decoder(params.codec_id);
            if (!decoder_ ) throw runtime_error("Unsupported codec");
            checkAlloc(codecContext_ = avcodec_alloc_context3(decoder_));
            if (avcodec_open2(codecContext_, decoder_, NULL) < 0)
                throw runtime_error("Can't open video codec");

            checkAlloc(frame_ = av_frame_alloc());
        }
        virtual ~MediaCtx() {
            avcodec_free_context(&codecContext_);
            av_frame_free(&frame_);
        }
    };

    struct VideoCtx : MediaCtx {
        typedef shared_ptr<VideoCtx> Ptr;
        double fps_ = 0;
        AVFrame* rgbFrame_ = NULL;
        struct SwsContext* scaler_ = NULL;

        VideoCtx(AVFormatContext* ctx, UDPFTPtr disp) : MediaCtx(ctx, AVMEDIA_TYPE_VIDEO) {
            const AVCodecParameters& params = * ctx->streams[index_]->codecpar;
            fps_ = av_q2d(ctx_->streams[index_]->avg_frame_rate);
            if (fps_ < 0)
                fps_ = 1.0 / av_q2d(ctx_->streams[index_]->codec->time_base);

            checkAlloc(rgbFrame_ = av_frame_alloc());

            int size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext_->width, codecContext_->height, 1);
            buffer_.resize((size_t) size);

            av_image_fill_arrays(rgbFrame_->data, rgbFrame_->linesize, buffer_.data(), AV_PIX_FMT_RGB24,
                                 codecContext_->width, codecContext_->height, 1);

            scaler_ = sws_getContext(codecContext_->width, codecContext_->height, codecContext_->pix_fmt,
                                     disp->width(), disp->height(), AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL );
            if (!scaler_)
                throw runtime_error("Video scaler failed to alloc");
        }
        ~VideoCtx() {
            av_frame_free(&rgbFrame_);
            sws_freeContext(scaler_);
        }
    };

    struct AudioCtx : MediaCtx {
        typedef shared_ptr<AudioCtx> Ptr;
        ao_device* device_ = NULL;

        AudioCtx(AVFormatContext* ctx) : MediaCtx(ctx, AVMEDIA_TYPE_AUDIO) {
            AVSampleFormat sfmt = codecContext_->sample_fmt;
            ao_sample_format format = {32, codecContext_->sample_rate, codecContext_->channels, AO_FMT_NATIVE, 0};
            format.bits = (sfmt==AV_SAMPLE_FMT_U8)? 8 : ((sfmt==AV_SAMPLE_FMT_S16)? 16 : 32);

            int deviceId = ao_default_driver_id();
            device_ = ao_open_live(deviceId, &format, NULL);
        }
        ~AudioCtx() {
            if (device_) ao_close(device_);
        }
    };


    void MediaPlayer::Init() {
        av_register_all();
        avformat_network_init();
        ao_initialize(); //audio output
    }

    MediaPlayer::MediaPlayer(string fname, Server& ctx, UDPFTPtr display) :
            fname_(fname),
            ctx_(ctx),
            display_(display),
            avCtx_(NULL)
    { }

    MediaPlayer::~MediaPlayer() {
        avformat_close_input(&avCtx_);
    }

    void MediaPlayer::showFrame(AVFrame *pFrame) {
        // Write pixel data
        const int height = display_->height();
        for(int y = 0; y < height; ++y) {
            char *raw_buffer = (char*) &display_->GetPixel(0, y); // Yes, I know :)
            memcpy(raw_buffer, pFrame->data[0] + y*pFrame->linesize[0],
                   3 * display_->width());
        }
        display_->Send();
    }

    void MediaPlayer::play() {
        // Open video file
        if (avformat_open_input(&avCtx_, fname_.c_str(), NULL, NULL) != 0)
            throw runtime_error("Can't open file " + fname_);

        ctx_.debug("Playing " + fname_);

        // Retrieve stream information
        if (avformat_find_stream_info(avCtx_, NULL) < 0)
            throw runtime_error("Can't open stream for " + fname_);

        // Dump information about file onto standard error
        if (ctx_.verbosity_ > 1)
            av_dump_format(avCtx_, 0, fname_.c_str(), 0);

        VideoCtx::Ptr video;
        AudioCtx::Ptr audio;
        try { video.reset(new VideoCtx(avCtx_, display_)); }
        catch (runtime_error e) { ctx_.debug(string(e.what())); }
        try { audio.reset(new AudioCtx(avCtx_)); }
        catch (runtime_error e) { ctx_.debug(string(e.what())); }


        // Read frames and send to FlaschenTaschen.
        const int frame_wait_micros = (int) (1e6 / (video? video->fps_ : 30));
        const double startTime = util::timeNow();
        long frame_count = 0, repeated_count = 0;
        while (ctx_.isRunning_) {
            frame_count = 0;
            AVPacket packet;

            int abuffer_size = 192000 + FF_INPUT_BUFFER_PADDING_SIZE;;
            uint8_t abuffer[abuffer_size];
            packet.data = abuffer;
            packet.size = abuffer_size;

            while (ctx_.isRunning_ && av_read_frame(avCtx_, &packet) >= 0) {
                int frameFinished = 0;
                // Is this a packet from the video stream?
                if (video && packet.stream_index == video->index_) {
                    //avcodec_send_packet(video->codecContext_, & video->packet_);

                    // Decode video frame
                    avcodec_decode_video2(video->codecContext_, video->frame_, &frameFinished, &packet);

                    if (frameFinished) { // Did we get a video frame?
                        // Convert the image from its native format to RGB
                        sws_scale(video->scaler_, (uint8_t const * const *)video->frame_->data,
                                  video->frame_->linesize, 0, video->codecContext_->height,
                                  video->rgbFrame_->data, video->rgbFrame_->linesize);

                        // Save the frame to disk
                        showFrame(video->rgbFrame_);
                        frame_count++;
                    }
                    usleep(frame_wait_micros);
                } else if (audio && (packet.stream_index == audio->index_)) {
                    int len = avcodec_decode_audio4(audio->codecContext_, audio->frame_, &frameFinished, &packet);
                    if (frameFinished)
                        ao_play(audio->device_, (char*)audio->frame_->extended_data[0], audio->frame_->linesize[0]);
                }

                // Free the packet that was allocated by av_read_frame
                av_free_packet(&packet);
            }
            repeated_count++; //if time allows- keep playing

            double elapsed = util::timeNow() - startTime;
            if (elapsed >= ctx_.minRepeatTime_)
                break;

            av_seek_frame(avCtx_, -1, 1, AVSEEK_FLAG_FRAME); //start playing from the beginning
            if (ctx_.verbosity_ > 1)
                cout << "loop " << repeated_count << " done after " << elapsed << "s (" << frame_count << " frames)" << endl;
        }

        avformat_close_input(&avCtx_);
        if (ctx_.verbosity_ > 1)
            cout << "Finished playing " << frame_count << " for " << (util::timeNow() - startTime) << "s total" << endl;
    }


}
