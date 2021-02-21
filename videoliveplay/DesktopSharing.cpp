#include "DesktopSharing.h"
#include "AudioCapture.h"
#include "net/NetInterface.h"
#include "net/Timestamp.h"
#include "xop/RtspServer.h"
#include "xop/H264Parser.h"


bool g_bUseADTS = true;

DesktopSharing::DesktopSharing()
	: _eventLoop(new xop::EventLoop)
{

}

DesktopSharing::~DesktopSharing()
{

}

DesktopSharing& DesktopSharing::instance()
{
	static DesktopSharing s_ds;
	return s_ds;
}

bool DesktopSharing::init(std::shared_ptr <IFrameCapture> p, IAudioCapture * p2)
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (_isInitialized)
		return false;

	_pFrameCapture = p;

	_pAudioCapture = p2;

	/* video config */
	_videoConfig.framerate = 25;
	_videoConfig.bitrate = 4000000;
	_videoConfig.gop = 25;

	if (_pFrameCapture && _pFrameCapture->init() < 0)
	{
		return false;
	}

    if (_pAudioCapture && _pAudioCapture->init() < 0)
    {
        return false;
    }


	if (_pAudioCapture)
	{
        /* audio config */
        _audioConfig.samplerate = _pAudioCapture->getSamplerate();
        _audioConfig.channels = _pAudioCapture->getChannels();
        _audioConfig.bitPreSample = _pAudioCapture->getBitsPerSample();

        if (!AACEncoder::instance().init(_audioConfig))
        {
            return false;
        }
	}

	if (_pFrameCapture)
	{
        _videoConfig.width = _pFrameCapture->getWidth();
        _videoConfig.height = _pFrameCapture->getHeight();

        if (!H264Encoder::instance().init(_videoConfig))
        {
            return false;
        }
	}

	std::thread t([this] {
		_eventLoop->loop();
	});
	t.detach();

	_isInitialized = true;
	return true;
}

void DesktopSharing::exit()
{
	if (_isRunning)
	{
		this->stop();
	}

	if (_isInitialized)
	{
		_isInitialized = false;
		if (_pFrameCapture)
		{
			_pFrameCapture->exit();
		}

		H264Encoder::instance().exit();
		_pAudioCapture->exit();
		AACEncoder::instance().exit();		
	}

	if (_rtspPusher!=nullptr && _rtspPusher->isConnected())
	{
		_rtspPusher->close();
		_rtspPusher = nullptr;
	}

	if (_rtmpPublisher != nullptr && _rtmpPublisher->isConnected())
	{
		_rtmpPublisher->close();
		_rtmpPublisher = nullptr;
	}

	if (_rtspServer != nullptr)
	{
		_eventLoop->quit();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		for (auto & _sessionId : _clients_sessionMap)
		{
			_rtspServer->removeMeidaSession(_sessionId.first);
		}
		_rtspServer = nullptr;
	}
}

void DesktopSharing::start()
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (!_isInitialized || _isRunning)
		return;
	
	_isRunning = true;

	if (_pFrameCapture && _pFrameCapture->start() == 0)
	{
		if (_pFrameCapture->isCapturing())
		{
			_videoThread.reset(new std::thread(&DesktopSharing::pushVideo, this));
		}
	}

	if (_pAudioCapture && _pAudioCapture->start() == 0)
	{
		if (_pAudioCapture->isCapturing())
		{
			_audioThread.reset(new std::thread(&DesktopSharing::pushAudio, this));
		}
	}
}

void DesktopSharing::stop()
{
	std::lock_guard<std::mutex> locker(_mutex);
	if (_isRunning)
	{
		_isRunning = false;
		if (_pFrameCapture->isCapturing())
		{
			_pFrameCapture->stop();
			_videoThread->join();
		}

		if (_pAudioCapture->isCapturing())
		{
			_pAudioCapture->stop();
			_audioThread->join();
		}	
	}
}

void DesktopSharing::startRtspServer(const std::set<std::string> & suffixSet, uint16_t rtspPort)
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (!_isInitialized)
	{
		return;
	}

	if (_rtspServer != nullptr)
	{
		return;
	}

	_ip = "0.0.0.0"; //xop::NetInterface::getLocalIPAddress(); //"127.0.0.1";
	_rtspServer.reset(new xop::RtspServer(_eventLoop.get(), _ip, rtspPort));

	for (const auto & suffix : suffixSet)
	{
		xop::MediaSession* session = xop::MediaSession::createNew(suffix);
		if (_pFrameCapture)
		{
			session->addMediaSource(xop::channel_0, xop::H264Source::createNew());
		}

		if (_pAudioCapture)
		{
			session->addMediaSource(xop::channel_1, xop::AACSource::createNew(_audioConfig.samplerate, _audioConfig.channels, g_bUseADTS));
		}

		session->setNotifyCallback([this](xop::MediaSessionId sessionId, uint32_t numClients) {
			_clients_sessionMap[sessionId] = numClients;
			//this->_clients = numClients;
		});

		xop::MediaSessionId _sessionId = _rtspServer->addMeidaSession(session);
		_clients_sessionMap[_sessionId] = 0;
		std::cout << "RTSP URL: " << "rtsp://" << xop::NetInterface::getLocalIPAddress() << ":" << std::to_string(rtspPort) << "/" << suffix << std::endl;
	}
}

void DesktopSharing::startRtspServer(std::string suffix, uint16_t rtspPort)
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (!_isInitialized)
	{
		return;
	}

	if (_rtspServer != nullptr)
	{
		return ;
	}

	_ip = "0.0.0.0"; //xop::NetInterface::getLocalIPAddress(); //"127.0.0.1";
	_rtspServer.reset(new xop::RtspServer(_eventLoop.get(), _ip, rtspPort));
	xop::MediaSession* session = xop::MediaSession::createNew(suffix);
	if (_pFrameCapture)
	{
		session->addMediaSource(xop::channel_0, xop::H264Source::createNew());
	}
	session->addMediaSource(xop::channel_1, xop::AACSource::createNew(_audioConfig.samplerate, _audioConfig.channels, g_bUseADTS));
	session->setNotifyCallback([this](xop::MediaSessionId sessionId, uint32_t numClients) {
		_clients_sessionMap[sessionId] = numClients;
	});

	xop::MediaSessionId _sessionId = _rtspServer->addMeidaSession(session);
	_clients_sessionMap[_sessionId] = 0;
	std::cout << "RTSP URL: " << "rtsp://" << xop::NetInterface::getLocalIPAddress() << ":" << std::to_string(rtspPort) << "/" << suffix << std::endl;
}

void DesktopSharing::startRtspPusher(const char* url)
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (!_isInitialized)
	{
		return;
	}

	if (_rtspPusher && _rtspPusher->isConnected())
	{
		_rtspPusher->close();
	}

	_rtspPusher.reset(new xop::RtspPusher(_eventLoop.get()));
	xop::MediaSession *session = xop::MediaSession::createNew();
	if (_pFrameCapture)
	{
		session->addMediaSource(xop::channel_0, xop::H264Source::createNew());
	}
	session->addMediaSource(xop::channel_1, xop::AACSource::createNew(_audioConfig.samplerate, _audioConfig.channels, g_bUseADTS));

	if (_rtspPusher->addMeidaSession(session) > 0)
	{
		if (_rtspPusher->openUrl(url) != 0)
		{
			_rtspPusher = nullptr;
			std::cout << "Open " << url << " failed." << std::endl;
			return;
		}		
	}

	std::cout << "Push rtsp stream to " << url << " ..." << std::endl;
}

void DesktopSharing::startRtmpPusher(const char* url)
{
	std::lock_guard<std::mutex> locker(_mutex);

	if (!_isInitialized)
	{
		return;
	}

	_rtmpPublisher.reset(new xop::RtmpPublisher(_eventLoop.get()));

	xop::MediaInfo mediaInfo;
	uint8_t* extradata = AACEncoder::instance().getAVCodecContext()->extradata;
	uint8_t extradata_size = AACEncoder::instance().getAVCodecContext()->extradata_size;

	mediaInfo.audioSpecificConfigSize = extradata_size;
	mediaInfo.audioSpecificConfig.reset(new uint8_t[mediaInfo.audioSpecificConfigSize]);
	memcpy(mediaInfo.audioSpecificConfig.get(), extradata, extradata_size);

	extradata = H264Encoder::instance().getAVCodecContext()->extradata;
	extradata_size = H264Encoder::instance().getAVCodecContext()->extradata_size;

	xop::Nal sps = xop::H264Parser::findNal(extradata, extradata_size);
	if (sps.first != nullptr && sps.second != nullptr && *sps.first == 0x67)
	{
		mediaInfo.spsSize = sps.second - sps.first + 1;
		mediaInfo.sps.reset(new uint8_t[mediaInfo.spsSize]);
		memcpy(mediaInfo.sps.get(), sps.first, mediaInfo.spsSize);

		xop::Nal pps = xop::H264Parser::findNal(sps.second, extradata_size - (sps.second - extradata));
		if (pps.first != nullptr && pps.second != nullptr && *pps.first == 0x68)
		{
			mediaInfo.ppsSize = pps.second - pps.first + 1;
			mediaInfo.pps.reset(new uint8_t[mediaInfo.ppsSize]);
			memcpy(mediaInfo.pps.get(), pps.first, mediaInfo.ppsSize);
		}
	}

	_rtmpPublisher->setMediaInfo(mediaInfo);

	if (_rtmpPublisher->openUrl(url, 2000) < 0)
	{
		std::cout << "Open url " << url << " failed." << std::endl;
		return;
	}

	std::cout << "Push rtmp stream to " << url << " ..." << std::endl;
}

void DesktopSharing::pushVideo()
{
	static xop::Timestamp tp, tp2;
	
	uint32_t fps = 0;
	uint32_t msec = 1000 / _videoConfig.framerate;
	tp.reset();

	while (this->_isRunning)
	{
		if (tp2.elapsed() >= 1000)
		{
			//printf("video fps: %d\n", fps); /*编码帧率统计*/
			tp2.reset();
			fps = 0;
		}

		uint32_t delay = msec;
		uint32_t elapsed = (uint32_t)tp.elapsed(); /*编码耗时计算*/
		if (elapsed > delay)
		{
			delay = 0;
		}
		else
		{
			delay -= elapsed;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		tp.reset();

		std::shared_ptr<uint8_t> bgraData;
		uint32_t bgraSize = 0;
		uint32_t timestamp = xop::H264Source::getTimeStamp();
		if (_pFrameCapture && _pFrameCapture->captureFrame(bgraData, bgraSize) == 0)
		{
			fps += 1;

			AVPacket* pkt = H264Encoder::instance().encodeVideo(bgraData.get(), _pFrameCapture->getWidth(), _pFrameCapture->getHeight());
			if (pkt)
			{
				xop::AVFrame vidoeFrame(pkt->size + 1024);
				vidoeFrame.size = 0;
				vidoeFrame.type = xop::VIDEO_FRAME_P;
				vidoeFrame.timestamp = timestamp;
				if (pkt->data[4] == 0x65 || pkt->data[4] == 0x6) //0x67:sps ,0x65:IDR, 0x6: SEI
				{
					// 编码器使用了AV_CODEC_FLAG_GLOBAL_HEADER, 这里需要添加sps, pps
					uint8_t* extraData = H264Encoder::instance().getAVCodecContext()->extradata;
					uint8_t extraDatasize = H264Encoder::instance().getAVCodecContext()->extradata_size;
					memcpy(vidoeFrame.buffer.get() + vidoeFrame.size, extraData + 4, extraDatasize - 4); // +4去掉H.264起始码
					vidoeFrame.size += (extraDatasize - 4);
					vidoeFrame.type = xop::VIDEO_FRAME_I;

					memcpy(vidoeFrame.buffer.get() + vidoeFrame.size, pkt->data, pkt->size);
					vidoeFrame.size += pkt->size;
				}
				else
				{
					memcpy(vidoeFrame.buffer.get() + vidoeFrame.size, pkt->data + 4, pkt->size - 4); // +4去掉H.264起始码
					vidoeFrame.size += (pkt->size - 4);
				}


				{

					for (const auto & iter : this->_clients_sessionMap)
					{
						// 本地RTSP视频转发
						if (_rtspServer != nullptr && iter.second > 0)
						{
							_rtspServer->pushFrame(iter.first, xop::channel_0, vidoeFrame);
						}
					}

                    // RTSP视频推流
                    if (_rtspPusher != nullptr && _rtspPusher->isConnected())
                    {
                        _rtspPusher->pushFrame(0, xop::channel_0, vidoeFrame);
                    }

					// RTMP视频推流
					if (_rtmpPublisher != nullptr && _rtmpPublisher->isConnected())
					{
						_rtmpPublisher->pushVideoFrame(vidoeFrame.buffer.get(), vidoeFrame.size);
					}
				}
			}
		}
	}
}

FILE* g_fp = nullptr;
FILE* g_fp_pcm = nullptr;

#define ADTS_HEADER_LEN  7;

void adts_header(char* szAdtsHeader, int dataLen) {

    int audio_object_type = 2;
    int sampling_frequency_index = 4;//4: 44100 Hz
    int channel_config = 2;

    int adtsLen = dataLen + 7;

    szAdtsHeader[0] = 0xff;         //syncword:0xfff                          高8bits
    szAdtsHeader[1] = 0xf0;         //syncword:0xfff                          低4bits
    szAdtsHeader[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    szAdtsHeader[1] |= (0 << 1);    //Layer:0                                 2bits
    szAdtsHeader[1] |= 1;           //protection absent:1                     1bit

    szAdtsHeader[2] = (audio_object_type - 1) << 6;            //profile:audio_object_type - 1                      2bits
    szAdtsHeader[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits
    szAdtsHeader[2] |= (0 << 1);                             //private bit:0                                      1bit
    szAdtsHeader[2] |= (channel_config & 0x04) >> 2;           //channel configuration:channel_config               高1bit

    szAdtsHeader[3] = (channel_config & 0x03) << 6;     //channel configuration:channel_config      低2bits
    szAdtsHeader[3] |= (0 << 5);                      //original：0                               1bit
    szAdtsHeader[3] |= (0 << 4);                      //home：0                                   1bit
    szAdtsHeader[3] |= (0 << 3);                      //copyright id bit：0                       1bit
    szAdtsHeader[3] |= (0 << 2);                      //copyright id start：0                     1bit
    szAdtsHeader[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    szAdtsHeader[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    szAdtsHeader[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    szAdtsHeader[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    szAdtsHeader[6] = 0xfc;
}

void GetAVPackData(std::vector<uint8_t> & datas, AVPacket* apt, bool bGetHeader = true)
{
	if (bGetHeader)
	{
		datas.resize(apt->size + 7);
        char adts_header_buf[7];
        adts_header(adts_header_buf, apt->size);
		memcpy(datas.data(), adts_header_buf, 7);
		memcpy(datas.data() + 7, apt->data, apt->size);
	}
	else
	{
		datas.resize(apt->size);
		memcpy(datas.data(), apt->data, apt->size);
	}
}

void DesktopSharing::pushAudio()
{
	if (!_pAudioCapture)
	{
		return;
	}

	std::shared_ptr<uint8_t> pcmBuf(new uint8_t[48000 * 8]);
	uint32_t frameSamples = AACEncoder::instance().getFrameSamples() * 4; //must be 4096
	uint32_t channel = _pAudioCapture->getChannels();
	uint32_t samplerate = _pAudioCapture->getSamplerate();

	while (this->_isRunning)
	{
		uint32_t timestamp = xop::AACSource::getTimeStamp(samplerate);
	
		if (_pAudioCapture->getSamples() >= (int)frameSamples)
		{
			if (_pAudioCapture->readSamples(pcmBuf.get(), frameSamples) != frameSamples)
			{
				continue;
			}

			if (!g_fp_pcm)
			{
				fopen_s(&g_fp_pcm, "wav2.pcm", "wb+");
			}

			std::vector<uint8_t> datas;
			AVPacket* pkt = AACEncoder::instance().encodeAudio(pcmBuf.get(), frameSamples, &datas);

			if (g_fp_pcm)
			{
                fwrite(pcmBuf.get(), 1, frameSamples, g_fp_pcm);
                fflush(g_fp_pcm);
			}

			if (pkt || !datas.empty())
			{
				if (datas.empty())
				{
					GetAVPackData(datas, pkt, g_bUseADTS);
				}

				xop::AVFrame audioFrame(datas.size());
				audioFrame.timestamp = timestamp;
				audioFrame.type = xop::AUDIO_FRAME;
				audioFrame.size = datas.size();
				memcpy(audioFrame.buffer.get(), datas.data(), datas.size());

				if (!g_fp)
				{
					fopen_s(&g_fp, "wav.aac", "wb+");
				}

				fwrite(datas.data(), 1, datas.size(), g_fp);
				fflush(g_fp);

				{

					for (const auto & iter : this->_clients_sessionMap)
					{
                        // RTSP音频转发
                        if (_rtspServer != nullptr)
                        {
                            _rtspServer->pushFrame(0, xop::channel_1, audioFrame);
                        }
					}

                    // RTSP音频推流
                    if (_rtspPusher && _rtspPusher->isConnected())
                    {
                        _rtspPusher->pushFrame(0, xop::channel_1, audioFrame);
                    }

					// RTMP音频推流
					if (_rtmpPublisher != nullptr && _rtmpPublisher->isConnected())
					{
						_rtmpPublisher->pushAudioFrame(audioFrame.buffer.get(), audioFrame.size);
					}
				}
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}