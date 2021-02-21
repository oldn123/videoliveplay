// CCaptureVideo视频捕捉类头文件
/////////////////////////////////////////////////////////////////////
#if !defined(AFX_CAPTUREVIDEO_H__F5345AA4_A39F_4B07_B843_3D87C4287AA0__INCLUDED_)
#define AFX_CAPTUREVIDEO_H__F5345AA4_A39F_4B07_B843_3D87C4287AA0__INCLUDED_
/////////////////////////////////////////////////////////////////////
// CaptureVideo.h : header file
/////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <atlbase.h>
#include <string>

#include "def.h"

using namespace std;




class CSampleGrabberCB;
struct IBaseFilter;
struct IGraphBuilder;
struct ICaptureGraphBuilder2;
struct IMediaControl;
struct IVideoWindow;
struct ISampleGrabber;

class VisCameraCatchExport CCaptureVideo
{
	friend class CSampleGrabberCB;
public:
	CCaptureVideo();
	virtual ~CCaptureVideo();

	static int EnumDevices();
	static int EnumDevice(){return EnumDevices();}

	HRESULT Init(int iDeviceIndex);
	void Release();
	void SetBindWnd(HWND hWnd){m_hWnd = hWnd;}
	void SetCallbackFunc(CaptureDecodeCallbackFunc_t fn, LPVOID puser);
	void GetVideoInfoHeader(BITMAPINFOHEADER &bmiHeader);

	bool GetSampleRate(int & lrate);

	HRESULT Preview();
	void ResizeVideoWindow();
	void Play();
	void StopPlay();

	void SetDeviceId(int nDevId);
	HRESULT ReInit();

	bool IsPlay() { return m_isPlay; }

//protected:
	void SwitchVideo(BOOL bMaster = TRUE);
	void SnapView(LPCTSTR strFile);
	void GrabOneFrame(BOOL bGrab);
	void GrabOneFrameToFile(BOOL bGrab, LPCTSTR strFile);
	HRESULT	CaptureImagesToAVI(const wchar_t * inFileName);//捕获保存视频
	
	//add by cixiaopan 20180409

private:
//	void FreeMediaType(AM_MEDIA_TYPE& mt);
	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	HRESULT SetupVideoWindow();
	HRESULT InitCaptureGraphBuilder();

	IBaseFilter *pMux;//捕获视频为AVI文件
	std::wstring		m_strOutFile;
	HWND m_hWnd;
	IGraphBuilder *m_pGB;
	ICaptureGraphBuilder2* m_pCapture;
	IBaseFilter* m_pBF;
	IMediaControl* m_pMC;
	IVideoWindow* m_pVW;
	ISampleGrabber* m_pGrabber;
	int m_ndevid;
	HWND m_hwnd;
	BITMAPINFOHEADER m_bmiHeader;
	CSampleGrabberCB *m_pSampleGrabberCB;
	bool m_isSetupVideoWindow;
	bool m_isPlay;
};
#endif // !defined(AFX_CAPTUREVIDEO_H__F5345AA4_A39F_4B07_B843_3D87C4287AA0__INCLUDED_)
