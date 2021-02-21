#pragma once

#include "CapInterface.h"
#include <windows.h>
#include <mutex>
#include "..\CameraCatch\Header\CameraCatchHeader.h"


class CCamCap2 : public IFrameCapture
{
public:
    CCamCap2();
    virtual ~CCamCap2();
    virtual int getWidth() override {
        return m_bmpInfo.biWidth;
    }

    virtual int getHeight() override {
        return m_bmpInfo.biHeight;
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
    int OnRealDataCallback(unsigned char* pBuffer, int nBufSize,
        int realDataType, void* realDataInfo);

private:
    CCaptureVideo*              m_pCap = nullptr;

    std::shared_ptr<uint8_t>    m_pVideoBuf = nullptr;
    int32_t                     m_nVideoBufLen = 0;

    bool                        m_bCapturing = false;
    BITMAPINFOHEADER            m_bmpInfo = {0};

    std::mutex                  m_mutex;
};


class CAudioCap : public IAudioCapture
{
public:
    CAudioCap();
    virtual ~CAudioCap();

    virtual int readSamples(uint8_t* data, uint32_t samples) override;
    virtual int getSamples() override;
    virtual int getSamplerate() override;
    virtual int getChannels() override;
    virtual int getBitsPerSample() override;
    virtual int init() override;
    virtual int start() override;
    virtual int stop() override;

    virtual bool isCapturing()const override {
        return m_bCapturing;
    }

    virtual int exit() override {
        return 0;
    }

private:
    CCaptureAudio* m_pAud = nullptr;

    std::shared_ptr<uint8_t>    m_pVideoBuf = nullptr;
    int32_t                     m_nVideoBufLen = 0;

    bool                        m_bCapturing = false;

    std::mutex                  m_mutex;
};
