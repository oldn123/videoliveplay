#pragma once

#include "..\..\..\Include\ffplayer.h"


#include <thread>
#include <memory>
#include <string>



class Audio;


class Player : public IRtspPlayer
{
public:
    Player() {}
	virtual ~Player() {};


public:
    virtual ffresurlt Init() override;
    virtual ffresurlt UnInit() override;
    virtual ffresurlt SetFile(const char*) override;
    virtual ffresurlt Play() override;
    virtual ffresurlt Stop() override;
    virtual ffresurlt SetBindWindow(HWND) override;
    virtual ffresurlt OnBindWindowMsgIdle() override;

	IMP_UNKNOWN;

public:


protected:
	std::string _videoAddr;
    HWND    _hParentWnd = nullptr;
    void* _pPlayData = nullptr;
};