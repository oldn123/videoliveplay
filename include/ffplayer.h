#pragma once

#include <windows.h>

// {497FACE3-FCF0-4356-AE3D-3E0B72E41983}
static const GUID IDL_IPLAYER =
{ 0x497face3, 0xfcf0, 0x4356, { 0xae, 0x3d, 0x3e, 0xb, 0x72, 0xe4, 0x19, 0x83 } };


typedef uint32_t ffresurlt;

class IGUnknown
{
public:
    virtual int32_t release() = 0;
};



#define IMP_UNKNOWN    virtual int32_t release() { delete this; return 0;}


class IVideoPlayer : public IGUnknown
{
public:
    virtual ffresurlt Init() = 0;
    virtual ffresurlt UnInit() = 0;
    virtual ffresurlt SetFile(const char*) = 0;
    virtual ffresurlt Play() = 0;
    virtual ffresurlt Stop() = 0;
    virtual ffresurlt SetBindWindow(HWND) = 0;
};

class IRtspPlayer : public IVideoPlayer
{
public:
};