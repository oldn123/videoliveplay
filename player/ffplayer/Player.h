#pragma once

#include "..\..\..\Include\ffplayer.h"

#include "AudioPacket.h"
#include <thread>
#include <memory>
#include <string>

struct AVFormatContext;
struct AVCodecParameters;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SwsContext;

class Audio;

class IBindAudio
{
public:
	virtual int getAudioPacket(AudioPacket*, AVPacket*, int) = 0;
};

class Player : public IRtspPlayer, public IBindAudio
{
public:
    Player() {}
	virtual ~Player() {};

public:
	void run(std::string, std::string window_name="");
	void clear();

public:
	virtual int getAudioPacket(AudioPacket*, AVPacket*, int) override;

public:
    virtual ffresurlt Init() override;
    virtual ffresurlt UnInit() override;
    virtual ffresurlt SetFile(const char*) override;
    virtual ffresurlt Play() override;
    virtual ffresurlt Stop() override;
	virtual ffresurlt SetPosition(int32_t, int32_t, int32_t, int32_t) override;
	IMP_UNKNOWN;

public:

	void open();

	int malloc(void);
	int display_video(void);
	int create_display(void);

	int get_video_stream(void);

	int read_audio_video_codec(void);

protected:
	void _PlayThread();
	void _LoopDecoder();
	void _PlayThread2();

	std::string video_addr;
	std::string window_name;

	int m_videoStream = -1;
	int m_audioStream = -1;

	AVFormatContext* pFormatCtx = NULL;

	AVCodecParameters* pCodecParameters = NULL;
	AVCodecParameters* pCodecAudioParameters = NULL;

	AVCodecContext* pCodecCtx = NULL;
	AVCodecContext* pCodecAudioCtx = NULL;

	AVCodec* pCodec = NULL;
	AVCodec* pAudioCodec = NULL;
	AVFrame* pFrame = NULL;
	AVFrame* pFrameRGB = NULL;

	uint8_t* buffer = NULL;

	SDL_Window* screen = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_Texture* bmp = NULL;

	struct SwsContext* sws_ctx = NULL;

	Audio* _pAudio = nullptr;

	int32_t _playStatus = 0;

	std::unique_ptr<std::thread>	_threadPlay = nullptr;
};