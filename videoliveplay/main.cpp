// 2019-8-15
// PHZ

// 支持本地RTSP转发和RTSP推流两种模式
// 支持RTMP推流
// 本地RTSP地址 rtsp://ip/live

#include "DesktopSharing.h"

#include "CCamCap2.h"
#include "AudioCapture.h"

#include <filesystem>
#include <thread>
#include <chrono>
#include <string>
#include <iosfwd>
#include <sstream> 

#include "..\Include\ffplayer.h"

#define RTSP_PUSH_TEST "rtsp://127.0.0.1:554/live" // RTSP推流地址, 在EasyDarwin下测试通过
#define RTMP_PUSH_TEST "rtmp://127.0.0.1/live/01"  // RTMP推流地址, 在SRS下测试通过

using namespace xop;

typedef int32_t(WINAPI *_CreateObject)(const GUID& guid, void** ppInterface);

void PlayThread()
{
    return;

	IRtspPlayer* pPlayer = nullptr;
	wchar_t buf[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buf, MAX_PATH);
	std::filesystem::path ph(buf);
	auto pt = ph.remove_filename();
	pt += L"ffplayer.dll";
	auto l = LoadLibraryEx(pt.wstring().c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (l)
	{
		_CreateObject pCreateObj = (_CreateObject)GetProcAddress(l, "CreateObject");
		if (pCreateObj)
		{
			pCreateObj(IDL_IPLAYER, (void**)&pPlayer);
			if (!pPlayer)
			{
				return;
			}
		}
	}

	if (1)
	{
        pPlayer->Init();
        //pPlayer->SetFile("rtsp://123.57.41.232/live");
        //pPlayer->SetFile(/*"rtsp://127.0.0.1:8554/live3"*/"F:\\BaiduNetdiskDownload\\wrq.mp4");
        pPlayer->SetFile("https://vod.pipi.cn/43903a81vodtransgzp1251246104/bbd4f07a5285890808066187974/v.f42906.mp4");
		//pPlayer->SetFile("D:\\Downloads\\H.264 interlaced_cut.mp4");
        pPlayer->Play();
	}

	MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))//Windows消息循环。
    {
        TranslateMessage(&msg);//翻译消息，如按键消息，翻译为WM_CHAR
        DispatchMessage(&msg);//分发消息到对应窗口
    }
}


void GetConfigInfo(std::string & sIp, bool & bCapAudio, bool & bCapVideo) {
    bCapAudio = true;
    bCapVideo = true;
    wchar_t sPath[MAX_PATH] = { 0 };
    GetModuleFileName(nullptr, sPath, MAX_PATH);
    wcscat(sPath, L".cfg");
    auto fp = _wfopen(sPath, L"rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        auto l = ftell(fp);
        char sbuf[1024] = { 0 };
        fseek(fp, 0, SEEK_SET);
        fread(sbuf, 1, l, fp);
        fclose(fp);
        sIp = sbuf;
    }

    if (sIp.empty())
    {
        sIp = "127.0.0.1";
    }
    else
    {

    }
}


int main(int argc, char **argv)
{
    bool bCapAudio = true;
    bool bCapVideo = true;
	std::string sIp;
	GetConfigInfo(sIp, bCapAudio, bCapVideo);

	std::stringstream ss;
	ss << "rtsp://" << sIp << ":554/live";


	LPVOID  pData = nullptr;
	CoInitialize(pData);

	std::thread th(PlayThread);

	if (1)
	{
        auto pc = std::make_shared<CCamCap2>();

		//auto & pa = AudioCapture::instance();
        //auto pc = std::make_shared<DXGIScreenCapture>();

        if (!bCapVideo)
        {
            pc = nullptr;
        }

        IAudioCapture* pac = nullptr;
        if (bCapAudio)
        {
            static CAudioCap pa;
            pac = &pa;
        }

        if (!DesktopSharing::instance().init(pc, pac))
        {
            getchar();
            return 0;
        }

        std::set<std::string> sset;
        sset.insert("live1");
        sset.insert("live2");
        sset.insert("live3");

        DesktopSharing::instance().start();
        DesktopSharing::instance().startRtspServer(sset, 8554);
        DesktopSharing::instance().startRtspPusher(ss.str().c_str()); //启动RTSP推流
        //DesktopSharing::instance().startRtmpPusher(RTMP_PUSH_TEST); //启动RTMP推流
	}


	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	DesktopSharing::instance().stop();
	DesktopSharing::instance().exit();
    getchar();


	th.join();

    return 0;
}

