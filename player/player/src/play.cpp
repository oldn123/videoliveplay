#include "..\include\ffmpegUtil.h"
#include <windows.h>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include "..\include\MediaProcessor.hpp"

extern "C"{
#include "SDL2/SDL.h"
};

#define REFRESH_EVENT (SDL_USEREVENT + 1)

#define BREAK_EVENT (SDL_USEREVENT + 2)

namespace {

    using namespace std;
    using namespace ffmpegUtil;

    void callback(void* userData, Uint8* stream, int len) {
        AudioProcessor* receiver = (AudioProcessor*)userData;
        receiver->writeAudioData(stream, len);
    }

    void readPkt(PacketGrabber& packetGrabber, AudioProcessor* audioProcessor, VideoProcessor* videoProcessor){
        const int CHECK_PERIOD = 10;

        cout << "read pkt thread started." << endl;
        int audioIndex = audioProcessor->getAudioIndex();
        int videoIndex = videoProcessor->getVideoIndex();

        while (!packetGrabber.isFileEnd() && !audioProcessor->isClosed() && !videoProcessor->isClosed()) {
            while (audioProcessor->needPacket() || videoProcessor->needPacket()) {
                AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
                int t = packetGrabber.grabPacket(packet);
                if (t == -1) {
                    cout << "file finish." << endl;
                    audioProcessor->pushPkt(nullptr);
                    videoProcessor->pushPkt(nullptr);
                    break;
                } else if (t == audioIndex && audioProcessor != nullptr) {
                    unique_ptr<AVPacket> uPacket(packet);
                    audioProcessor->pushPkt(std::move(uPacket));
                } else if (t == videoIndex && videoProcessor != nullptr) {
                    unique_ptr<AVPacket> uPacket(packet);
                    videoProcessor->pushPkt(std::move(uPacket));
                } else {
                    av_packet_free(&packet);
                    cout << "unknown streamIndex:" << t << endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_PERIOD));
        }
        cout << "read pkt thread finished." << endl;
    }

    bool refreshFrame(SDL_Renderer* sdlRenderer, SDL_Texture* sdlTexture, VideoProcessor & videoProcessor, AudioProcessor* audio, 
        bool & faster, int & slowCount, int & fastCount, int & failCount) {
        if (audio != nullptr) {
            auto vTs = videoProcessor.getPts();
            auto aTs = audio->getPts();
            if (vTs > aTs && vTs - aTs > 30) {
                cout << "VIDEO FASTER ================= vTs - aTs [" << (vTs - aTs)
                    << "]ms, SKIP A EVENT" << endl;
                faster = false;
                slowCount++;
                return false ;
            }
            else if (vTs < aTs && aTs - vTs > 30) {
                cout << "VIDEO SLOWER ================= aTs - vTs =[" << (aTs - vTs) << "]ms, Faster" << endl;
                faster = true;
                fastCount++;
            }
            else {
                faster = false;
            }
        }

        AVFrame* frame = videoProcessor.getFrame();

        if (frame != nullptr) {
            SDL_UpdateYUVTexture(sdlTexture, NULL,

                frame->data[0], frame->linesize[0],

                frame->data[1], frame->linesize[1],

                frame->data[2], frame->linesize[2]);

            SDL_RenderClear(sdlRenderer);
            SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(sdlRenderer);

            if (!videoProcessor.refreshFrame()) {
                cout << "vProcessor.refreshFrame false" << endl;
            }
        }
        else {
            failCount++;
            cout << "getFrame fail. failCount = " << failCount << endl;
        }

        return true;
    }

    class CSDLPlay
    {
    public:
        static CSDLPlay * play(const string& inputPath, HWND h) {            
            CSDLPlay* pPlay = new CSDLPlay;
            pPlay->playVideoAndAudio(inputPath, h);
            return pPlay;
        }

        static void close(CSDLPlay* pPlay) {
            if (pPlay)
            {
                pPlay->closePlay();
                delete pPlay;
            }         
        }

    public:
        int closePlay() {
            SDL_PauseAudioDevice(m_audioDeviceId, 1);
            SDL_CloseAudio();

            SDL_Quit();

            auto r = m_spAudioProcessor->close();
            r = m_spVideoProcessor->close();
            m_spReaderThread->join();

            if (sdlTexture)
                SDL_DestroyTexture(sdlTexture);

            if (sdlRenderer)
                SDL_DestroyRenderer(sdlRenderer);

            if (window)
                SDL_DestroyWindow(window);

            return 0;
        }

        bool initPlay(const char * inputPath) {

            if (!m_spPacketGrabber)
            {
                m_spPacketGrabber = std::make_shared<PacketGrabber>(inputPath);
            }

            auto formatCtx = m_spPacketGrabber->getFormatCtx();
            av_dump_format(formatCtx, 0, "", 0);

            m_spVideoProcessor = std::make_shared<VideoProcessor>(formatCtx);
            m_spVideoProcessor->start();

            m_spAudioProcessor = std::make_shared<AudioProcessor>(formatCtx);
            m_spAudioProcessor->start();

            return true;
        }

        void refreshPicture() {
            while (!exit) {
                SDL_Event event;
                event.type = REFRESH_EVENT;
                SDL_PushEvent(&event);
                int time = 1000 / m_spVideoProcessor->getFrameRate();
                if (faster) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(time / 2));
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(time));
                }
            }
        }

        bool onIdle(){
            SDL_Event event;
            if (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT) {
                    cout << "SDL screen got a SDL_QUIT." << endl;
                    exit = true;
                    return false;
                }
                else if (event.type == BREAK_EVENT) {
                    return false;
                }
                else if (event.type == REFRESH_EVENT)
                {
                    refreshFrame(sdlRenderer, sdlTexture, *m_spVideoProcessor, m_spAudioProcessor.get(),
                        faster, slowCount, fastCount, failCount);
                }
            }
    
            return true;
        }

        void loopEvt()
        {
            while (!exit)
            {
                if (!onIdle())
                {
                    break;
                }
            }      
        }

        auto videoPlay(VideoProcessor& videoProcessor, AudioProcessor* audio = nullptr, HWND hWnd = nullptr) {

            std::shared_ptr<std::thread>    videoThread;

            auto width = 100;
            auto height = 100;
            bool bVideoOk = false;
            if (videoProcessor.getVideoIndex() >= 0)
            {
                width = videoProcessor.getWidth();
                height = videoProcessor.getHeight();
                bVideoOk = true;
            }

            if (!hWnd)
            {
                window = SDL_CreateWindow("player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
            }
            else
            {
                window = SDL_CreateWindowFrom(hWnd);
            }

            if (!window) {
                string errMsg = "could not create window";
                errMsg += SDL_GetError();
                cout << errMsg << endl;
                throw std::runtime_error(errMsg);
            }

            auto wid = SDL_GetWindowID(window);

            sdlRenderer = SDL_CreateRenderer(window, -1, 0);

            Uint32 pixFormat = SDL_PIXELFORMAT_IYUV;

            sdlTexture = SDL_CreateTexture(sdlRenderer, pixFormat, SDL_TEXTUREACCESS_STREAMING, width, height);

            if (bVideoOk)
            {
                videoThread = std::make_shared<std::thread>(&CSDLPlay::refreshPicture, this);

                if (!hWnd)
                {
                    loopEvt();
                }
            }

            return videoThread;
        }

        void audioPlay() {
            SDL_AudioDeviceID& audioDeviceId = m_audioDeviceId;
            AudioProcessor& audioProcessor = *m_spAudioProcessor;

            SDL_AudioSpec spec;
            SDL_AudioSpec wantedSpec;

            cout << "audioProcessor.getSampleFormat()" << audioProcessor.getSampleFormat() << endl;
            cout << "audioProcessor.getOutSampleRate()" << audioProcessor.getOutSampleRate() << endl;
            cout << "audioProcessor.getOutChannels()" << audioProcessor.getOutChannels() << endl;

            int samples = -1;

            while (true) {
                cout << "get audio samples" << endl;
                samples = audioProcessor.getSamples();
                if (samples <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                else {
                    cout << "get audio samples" << samples << endl;
                    break;
                }
            }

            wantedSpec.freq = audioProcessor.getOutSampleRate();
            wantedSpec.format = AUDIO_S16SYS;
            wantedSpec.channels = audioProcessor.getOutChannels();
            wantedSpec.samples = samples;
            wantedSpec.callback = callback;
            wantedSpec.userdata = &audioProcessor;

            audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &spec, 0);

            if (audioDeviceId == 0) {
                string errMsg = "Failed to open audio device:";
                errMsg += SDL_GetError();
                cout << errMsg << endl;
                throw std::runtime_error(errMsg);
            }

            cout << "wantedSpec.freq:" << wantedSpec.freq << endl;
            std::printf("wantedSpec.format: Ox%X\n", wantedSpec.format);
            cout << "wantedSpec.channels:" << wantedSpec.channels << endl;
            cout << "wantedSpec.samples:" << wantedSpec.samples << endl;

            cout << "spec.freq:" << spec.freq << endl;
            std::printf("spec.format: Ox%X\n", spec.format);
            cout << "spec.channels:" << spec.channels << endl;
            cout << "spec.silence:" << spec.silence << endl;
            cout << "spec.samples:" << spec.samples << endl;

            SDL_PauseAudioDevice(audioDeviceId, 0);
            cout << "audio start thread finish." << endl;
        }

        int playVideoAndAudio(const string& inputPath, HWND h) {

            initPlay(inputPath.c_str());
            
            m_spReaderThread = std::make_shared<std::thread>(readPkt, std::ref(*m_spPacketGrabber),
                m_spAudioProcessor.get(),
                m_spVideoProcessor.get());

            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);

            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
                string errMsg = "Could not initialize SDL -";
                errMsg += SDL_GetError();
                cout << errMsg << endl;
                throw std::runtime_error(errMsg);
            }

            std::thread startAudioThread(&CSDLPlay::audioPlay, this);
            startAudioThread.join();

            m_spVideoPlay = videoPlay(*m_spVideoProcessor, m_spAudioProcessor.get(), h);

            if (!h)
            {
                if (m_spVideoPlay)
                {
                    m_spVideoPlay->join();
                }

                closePlay();
            }

            return 0;
        }
    protected:
        std::shared_ptr<PacketGrabber>  m_spPacketGrabber;

        std::shared_ptr<VideoProcessor> m_spVideoProcessor;

        std::shared_ptr<AudioProcessor> m_spAudioProcessor;

        std::shared_ptr<std::thread>    m_spReaderThread;

        std::shared_ptr<std::thread>    m_spVideoPlay;

        SDL_AudioDeviceID m_audioDeviceId;

        SDL_Window* window = nullptr;
        SDL_Renderer* sdlRenderer = nullptr;
        SDL_Texture* sdlTexture = nullptr;


        bool exit = false;
        bool faster = false;
        int failCount = 0;
        int fastCount = 0;
        int slowCount = 0;
    };
}


void * play(const string& inputPath, HWND h){
    return (void *)CSDLPlay::play(inputPath, h);
}

void close(void * p) {
    CSDLPlay* pP = (CSDLPlay*)p;
    CSDLPlay::close(pP);
}

int onIdle(void* p) {
    CSDLPlay* pP = (CSDLPlay*)p;
    return pP->onIdle();
}