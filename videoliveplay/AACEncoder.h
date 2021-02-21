#ifndef _AAC_ENCODER_H
#define _AAC_ENCODER_H

#include <cstdint>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/mem.h>
#include <libavutil/fifo.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

#include <vector>

//#include ".\aacenc\PcmToAac.h"

struct AudioConfig
{
	uint32_t channels = 2;
	uint32_t samplerate = 48000;
	uint32_t bitrate = 128 * 1024; //16000 * 4;
	uint32_t bitPreSample = 0;
};


class AACEncoder
{
public:
	AACEncoder & operator=(const AACEncoder &) = delete;
	AACEncoder(const AACEncoder &) = delete;
	static AACEncoder& instance();
	~AACEncoder();

	bool init(struct AudioConfig ac);
	void exit();

	uint32_t getFrameSamples();

	AVCodecContext* getAVCodecContext() const
	{ return _aCodecCtx; }

	AVPacket* encodeAudio(const uint8_t *pcm, int samples, std::vector<uint8_t> * pOutData = nullptr);
	AVPacket* encodeAudio2(const uint8_t* pcm, int samples);
private:
	AACEncoder();

	bool _isInitialized = false;
	AudioConfig _audioConfig;

	AVCodecContext *_aCodecCtx = nullptr;
	SwrContext* _swrCtx = nullptr;
	AVFrame* _pcmFrame = nullptr;
	AVPacket* _aPkt = nullptr;
	uint32_t _aPts = 0;

#ifdef _FAAC_ENCODER_H_
	FAACEncoder m_faacEnc;
#endif

#ifdef MUXER_PCMTOAAC_H
	PcmToAac m_faacEnc;
#endif
	

};

#endif