#ifndef RTCVIDEOSINK_H
#define RTCVIDEOSINK_H
#include <mutex>
#include "VideoRenderer.h"
class RtcVideoSink {
public:
	RtcVideoSink();
	~RtcVideoSink();
	void StartRenderer();
	void StopRenderer();
	bool setVideoWindow(void * wnd);
	void setMirror(bool useMirror);
	bool renderYuv(uint8_t *data, int width, int height);
    bool renderYuv(int width, int height, const uint8_t* Yplane, int Ypitch,
        const uint8_t* Uplane, int Upitch,
        const uint8_t* Vplane, int Vpitch);
private:
	void*							m_hWnd = 0;		/// window to render video
	uint64_t					    m_count = 0;		/// number of video frames
	IRenderer*				        m_sdkRenderer = nullptr;	/// render as sdk
	bool                            m_startRender = true;
	std::mutex                      m_mutexObserver;
};

#endif // RTCVIDEOSINK_H