#pragma once

#include <memory>

class IFrameCapture
{
public:
    virtual ~IFrameCapture() {};
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual int captureFrame(std::shared_ptr<uint8_t>& bgraPtr, uint32_t& size) = 0;

    virtual bool isCapturing()const = 0;
    virtual int init(int displayIndex = 0) = 0;
    virtual int exit() = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
};


class IAudioCapture
{
public:
    virtual ~IAudioCapture() {};

    virtual int readSamples(uint8_t* data, uint32_t samples) = 0;
    virtual int getSamples() = 0;
    virtual int getSamplerate() = 0;
    virtual int getChannels() = 0;
    virtual int getBitsPerSample() = 0;

    virtual bool isCapturing()const = 0;
    virtual int init() = 0;
    virtual int exit() = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
};
