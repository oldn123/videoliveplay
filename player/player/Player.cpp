#include "stdafx.h"
#include "player.h"
#include ".\src\player.hpp"


using namespace std;


ffresurlt Player::Init()
{
    SDLWrapper::init_sdl();
	return 0;
}

ffresurlt Player::UnInit()
{
	return 0;
}

ffresurlt Player::SetFile(const char* sfile)
{
	if (sfile)
	{
		_videoAddr = sfile;
	}
	return 0;
}

ffresurlt Player::Play()
{
	_pPlayData = play(_videoAddr, _hParentWnd);
	return 0;
}

ffresurlt Player::Stop()
{
	close(_pPlayData);
	return 0;
}

ffresurlt Player::SetBindWindow(HWND H)
{
    _hParentWnd = H;
    return 0;
}

ffresurlt Player::OnBindWindowMsgIdle() 
{
	return onIdle(_pPlayData);
}
