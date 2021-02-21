#pragma once

#include "CapInterface.h"
#include <windows.h>
#include <mutex>

class CCamCap : public IFrameCapture
{
public:
    CCamCap();
    virtual ~CCamCap();
    virtual int getWidth() override {
        return m_iWidth;
    }

    virtual int getHeight() override {
        return m_iHeight;
    }

    virtual bool isCapturing()const override {
        return m_bCapturing;
    }

    virtual int exit() override {
        return 0;
    }
    virtual int init(int displayIndex = 0) override;
    virtual int start() override;
    virtual int stop() override;
    virtual int captureFrame(std::shared_ptr<uint8_t>& bgraPtr, uint32_t& size) override;

public:
    int OnRealDataCallback(int nDevId, unsigned char* pBuffer, int nBufSize,
        RealDataStreamType realDataType, void* realDataInfo);

private:
    DShowCaptureVideo_Interface* m_pCapVideo = nullptr;
    DShowCaptureAudio_Interface* m_pCapAudio = nullptr;

    std::shared_ptr<uint8_t>    m_pVideoBuf = nullptr;
    int32_t                     m_nVideoBufLen = 0;

    bool                        m_bCapturing = false;
    int32_t                     m_iWidth = 0;
    int32_t                     m_iHeight = 0;
    int32_t                     m_iBits = 0;

    std::mutex                  m_mutex;
};

