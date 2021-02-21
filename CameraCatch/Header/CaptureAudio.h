/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
// CaptureAudio.h: interface for the CCaptureAudio class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAPTUREAUDIO_H__4FD3D541_79D0_47AB_891C_BFE4FE9EA54A__INCLUDED_)
#define AFX_CAPTUREAUDIO_H__4FD3D541_79D0_47AB_891C_BFE4FE9EA54A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "DshowDef.h"

#include "def.h"
#include <thread>
#include <memory>
#include <list>
#include <vector>
#include <mutex>
#include <atomic>
using namespace std;


class CSampleGrabberCB;
struct IGraphBuilder;//filter grap 由外部传递
struct ICaptureGraphBuilder2;
struct IBaseFilter;//设备的Filter;
struct IVideoWindow; //视频显示窗口
struct IMediaControl;//媒体控制
struct IBaseFilter;
struct ISampleGrabber;

class VisCameraCatchExport CCaptureAudio
{

public:
	CCaptureAudio();
	virtual ~CCaptureAudio();

private:
	IGraphBuilder *m_pGraphBuilder;//filter grap 由外部传递
	ICaptureGraphBuilder2* m_pCaptureGraphBulid;
	IBaseFilter* m_pBaseFilter;//设备的Filter;
	IVideoWindow* m_pVideoWin; //视频显示窗口
	IMediaControl* m_pMediaCon;//媒体控制
	IBaseFilter *m_pSampleGrabberFilter;
	ISampleGrabber *m_pSampleGrabber;

private:
	int m_nChannels = 0;
	int m_nBytesPerSample = 16;//位数16位
	int m_nSampleRate = 4410;//44100;,采样率
	int	m_nAudioBufferType = 0;
	int m_nPinType = 0;
	int m_iDeviceId = 0;
    CSampleGrabberCB* m_pSampleGrabberCB;


private:
	std::shared_ptr<uint8_t[]> m_spDataBuffer = nullptr;
	int m_nBufferSize = 0;

private:
	HRESULT CreateCaptureGraphBuilder();
	BOOL BindToAudioDev(int deviceId, IBaseFilter **pFilter);
	HRESULT SetAudioFormat(int nChannels,int nBytesPerSample,long nFrequency,int nAudioBufferType,int nPinType);
	HRESULT CreateCaptureSampleGrabber();
	HRESULT StartPreview();
	HRESULT StopPreview();
	void TearDownGraph();
	void NukeDownstream(IGraphBuilder * inGraph, IBaseFilter * inFilter);

public:
	bool Init();
	int BulidCaptureGraph();

	int GetCurrentBuffer(uint8_t* pbuf, uint32_t lDatasize);
	int getSamples();
	int getSamplerate();
	int getChannels();
	int getBitsPerSample();

	int stop();

	void	OnBufferCallback(uint8_t * pbuf, int32_t nlen);

private:
	std::shared_ptr<std::thread> m_threadptr = nullptr;

	std::list<std::vector<uint8_t>>	m_dataList;
	uint32_t	m_dataSize = 0;
	std::mutex	m_muxData;
};

#endif // !defined(AFX_CAPTUREAUDIO_H__4FD3D541_79D0_47AB_891C_BFE4FE9EA54A__INCLUDED_)
