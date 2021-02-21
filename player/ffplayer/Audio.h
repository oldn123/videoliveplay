#pragma once
#include "AudioPacket.h"
#include "AudioCallback.h"
class IBindAudio;

class Audio
{
public:
	struct SwrContext* swrCtx = NULL;
	AVFrame wanted_frame;

	AudioPacket audioq;

	void open();
	void malloc(AVCodecContext*);
	void init_audio_packet(AudioPacket*);
	int audio_decode_frame(AVCodecContext*, uint8_t*, int);
	int put_audio_packet(AVPacket*);

public:
	Audio(IBindAudio* p) { m_pBindAudio = p; }
	virtual ~Audio() {

	}
private:
	IBindAudio* m_pBindAudio = nullptr;

	SDL_AudioSpec wantedSpec = { 0 }, audioSpec = { 0 };
};