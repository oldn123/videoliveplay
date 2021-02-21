﻿#ifndef DESKTOP_SHARING_H
#define DESKTOP_SHARING_H

#include <mutex>
#include <set>
#include <map>
#include "xop/RtspServer.h"
#include "xop/RtspPusher.h"
#include "xop/RtmpPublisher.h"
#include "net/Timer.h"
#include "H264Encoder.h"
#include "AACEncoder.h"
#include "ScreenCapture/DXGIScreenCapture.h"

class DesktopSharing
{
public:
	DesktopSharing & operator=(const DesktopSharing &) = delete;
	DesktopSharing(const DesktopSharing &) = delete;
	static DesktopSharing& instance();
	~DesktopSharing();

	bool init(std::shared_ptr <IFrameCapture>, IAudioCapture *);
	void exit();

	void start();
	void stop();

	void startRtspServer(const std::set<std::string> & suffixSet, uint16_t rtspPort = 554);
	void startRtspServer(std::string suffix = "live", uint16_t rtspPort = 554);
	void startRtspPusher(const char* url);
	void startRtmpPusher(const char* url);

private:
	DesktopSharing();
	void pushAudio();
	void pushVideo();

	std::string _ip;

	bool _isInitialized = false;
	bool _isRunning = false;

	std::map<xop::MediaSessionId, uint32_t>	_clients_sessionMap;

//	uint32_t _clients = 0;
//	xop::MediaSessionId _sessionId = 0;
	std::shared_ptr<xop::EventLoop> _eventLoop = nullptr;
	std::shared_ptr<xop::RtspServer> _rtspServer = nullptr;	
	std::shared_ptr<xop::RtspPusher> _rtspPusher = nullptr;
	std::shared_ptr<xop::RtmpPublisher> _rtmpPublisher = nullptr;
	AudioConfig _audioConfig;
	VideoConfig _videoConfig;

	std::mutex _mutex;
	std::shared_ptr<std::thread> _videoThread;
	std::shared_ptr<std::thread> _audioThread;

	std::shared_ptr <IFrameCapture> _pFrameCapture = nullptr;
	IAudioCapture * _pAudioCapture = nullptr;
	//DXGIScreenCapture _screenCapture;
};

#endif

