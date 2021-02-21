#include "stdafx.h"
// CaptureVideo.cpp: implementation of the CCaptureVideo class.
//
//////////////////////////////////////////////////////////////////////

#include <atlconv.h>

#include "../header/cb.hpp"


#include "../Header/CaptureVideo.h"
#include "../Header/CaptureAudio.h"


#pragma warning(disable : 4995)
typedef BOOL (WINAPI *PRTWND)(HWND, HDC, WORD);


BOOL SaveBitmapToFile(HBITMAP hBitmap, LPSTR lpFileName)
{
	HDC hDC;						// �豸������

	int iBits;						// ��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
	WORD wBitCount;					// λͼ��ÿ��������ռ�ֽ���
	DWORD dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;	// ��ɫ���С��λͼ���ݴ�С��λͼ�ļ���С��д���ļ��ֽ���
	BITMAP Bitmap;					//λͼ���Խṹ
	BITMAPFILEHEADER bmfHdr;		// λͼ�ļ�ͷ
	BITMAPINFOHEADER bi;			// λͼ��Ϣͷ
	LPBITMAPINFOHEADER lpbi;		// ָ��λͼ��Ϣͷ�ṹ

	HANDLE fh, hDib;				// �����ļ��������ڴ���
	HPALETTE hPal, hOldPal=NULL;	// ��ɫ����

	// ����λͼ�ļ�ÿ��������ռ�ֽ���
	hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else if (iBits <= 24)
		wBitCount = 24;
	else
		wBitCount = 32;
	if (wBitCount <= 8)
		dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD);		// �����ɫ���С

	// ����λͼ��Ϣͷ�ṹ
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));	// Ϊλͼ���ݷ����ڴ�
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;
	// �����ɫ��
	hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = GetDC(NULL);
		hOldPal = SelectPalette(hDC, hPal, FALSE);
		RealizePalette(hDC);
	}
	// ��ȡ�õ�ɫ�����µ�����ֵ
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO*)lpbi, DIB_RGB_COLORS);

	if (hOldPal)				// �ָ���ɫ��
	{
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(NULL, hDC);
	}
	// ����λͼ�ļ� 
	USES_CONVERSION;
	fh = CreateFile(A2T(lpFileName), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;

	// ����λͼ�ļ�ͷ
	bmfHdr.bfType = 0x4D42;		// �ļ�����: "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize; 
	bmfHdr.bfSize = dwDIBSize;	// λͼ�ļ���С
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);	// д��λͼ�ļ�ͷ
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);					// д��λͼ�ļ���������

	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return TRUE;
}


HBITMAP ScreenCapture(LPSTR filename,WORD BitCount,LPRECT lpRect)
{
	HBITMAP hBitmap;
	//��ʾ����ĻDC
	HDC hScreenDC=CreateDC(L"DISPLAY",NULL,NULL,NULL);
	HDC hmenDC=CreateCompatibleDC(hScreenDC);
	//��ʾ����Ļ�Ŀ�͸�
	int ScreenWidth=GetDeviceCaps(hScreenDC,HORZRES);
	int ScreenHeight=GetDeviceCaps(hScreenDC,VERTRES);

	HBITMAP hOldBM;
	//����λͼ����
	PVOID lpvpxldata;
	//��ȡ��ȡ�ĳ��ȼ����
	INT ixStart;
	INT iyStart;
	INT iX;
	INT iY;
	//λͼ���ݴ�С
	DWORD dwBitmapArraySize;

	DWORD nBitsOffset;
	DWORD lImageSize;
	DWORD lFileSize;

	BITMAPINFO bmInfo;

	BITMAPFILEHEADER bmFileHeader;
	HANDLE hbmfile;
	DWORD dwWritten;

	if(lpRect==NULL)
	{
		ixStart=iyStart=0;
		iX=ScreenWidth;
		iY=ScreenHeight;
	}
	else
	{
		ixStart=lpRect->left;
		iyStart=lpRect->top;
		iX=lpRect->right - lpRect->left;
		iY=lpRect->bottom - lpRect->top;
	}
	hBitmap=CreateCompatibleBitmap(hScreenDC,iX,iY);
	hOldBM=(HBITMAP)SelectObject(hmenDC,hBitmap);
	BitBlt(hmenDC,0,0,iX,iY,hScreenDC,ixStart,iyStart,SRCCOPY);
	hBitmap=(HBITMAP)SelectObject(hmenDC,hOldBM);
	if(filename==NULL)
	{
		DeleteDC(hScreenDC);
		DeleteDC(hmenDC);
		return hBitmap;
	}

	dwBitmapArraySize = ((((iX*32)+31)&~31)>>3)*iY;
	lpvpxldata=HeapAlloc(GetProcessHeap(),HEAP_NO_SERIALIZE,dwBitmapArraySize);
	ZeroMemory(lpvpxldata,dwBitmapArraySize);

	ZeroMemory(&bmInfo,sizeof(BITMAPINFO));
	bmInfo.bmiHeader.biSize=sizeof(PBITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth=iX;
	bmInfo.bmiHeader.biHeight=iY;
	bmInfo.bmiHeader.biPlanes=1;
	bmInfo.bmiHeader.biBitCount=BitCount;
	bmInfo.bmiHeader.biClrImportant=BI_RGB;

	ZeroMemory(&bmFileHeader,sizeof(BITMAPFILEHEADER));
	nBitsOffset=sizeof(BITMAPFILEHEADER)+bmInfo.bmiHeader.biSize;
	lImageSize=((((bmInfo.bmiHeader.biWidth*bmInfo.bmiHeader.biBitCount)+31)& ~31)>>3)*bmInfo.bmiHeader.biHeight;
	lFileSize=nBitsOffset+lImageSize;
	bmFileHeader.bfOffBits=nBitsOffset;

	GetDIBits(hmenDC,hBitmap,0,bmInfo.bmiHeader.biHeight,lpvpxldata,&bmInfo,DIB_RGB_COLORS);
	USES_CONVERSION;
	hbmfile=CreateFile(A2T(filename),GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if(hbmfile==INVALID_HANDLE_VALUE)
	{
		//MessageBox(NULL,"create file error","error",MB_OK);
	}
	WriteFile(hbmfile,&bmFileHeader,sizeof(BITMAPCOREHEADER),&dwWritten,NULL);
	WriteFile(hbmfile,&bmInfo,sizeof(BITMAPINFO),&dwWritten,NULL);
	WriteFile(hbmfile,lpvpxldata,lImageSize,&dwWritten,NULL);
	CloseHandle(hbmfile);


	HeapFree(GetProcessHeap(),HEAP_NO_SERIALIZE,lpvpxldata);
	ReleaseDC(0,hScreenDC);
	DeleteDC(hmenDC);
	return hBitmap;
}



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif
BOOL bOneShot=FALSE;//ȫ�ֱ���
wchar_t szFileName[MAX_PATH];// λͼ�ļ����� 
//CEvent g_evtSnap;

void FreeMediaType(AM_MEDIA_TYPE& mt);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCaptureVideo::CCaptureVideo()
{
	//COM Library Intialization
	//if(FAILED(CoInitialize(NULL))) /*, COINIT_APARTMENTTHREADED)))*/
	//	return;

	m_ndevid = -1;
	m_pBF = NULL;
	m_hWnd = NULL;
	m_pVW = NULL;
	m_pMC = NULL;
	m_pGB = NULL;
	m_pCapture = NULL; 
	m_pGrabber = NULL;
	m_pSampleGrabberCB = new CSampleGrabberCB;
	m_isSetupVideoWindow = false;
	m_isPlay = false;
}

CCaptureVideo::~CCaptureVideo()
{
	// Stop media playback
	Release();
	//CoUninitialize();
	delete m_pSampleGrabberCB;

}

void CCaptureVideo::SetCallbackFunc(CaptureDecodeCallbackFunc_t fn, LPVOID puser)
{
	m_pSampleGrabberCB->m_fn = fn;
	m_pSampleGrabberCB->m_cxt = puser;
}

void CCaptureVideo::Release()
{
	if(m_pMC)
	{
		m_pMC->Stop();
	}
		
	if(m_pVW)
	{
		m_pVW->put_Visible(OAFALSE);
		m_pVW->put_Owner(NULL);
	}

	SAFE_RELEASE(m_pCapture);
	SAFE_RELEASE(m_pMC);
	SAFE_RELEASE(m_pGB);
	SAFE_RELEASE(m_pBF);
	SAFE_RELEASE(m_pVW);
	SAFE_RELEASE(m_pGrabber);
	m_pVW = NULL;
	m_pMC = NULL;
	m_pGB = NULL;
	m_pCapture = NULL; 
	m_isSetupVideoWindow = false;
	m_isPlay = false;
}

HRESULT CCaptureVideo::ReInit()
{
	Release();

	return Init(m_ndevid);
}

/*���ò�����Ƶ���ļ�����ʼ��׽��Ƶ����д�ļ�*/
HRESULT CCaptureVideo::CaptureImagesToAVI(const wchar_t* inFileName)
{
	HRESULT hr=0;
	m_pMC->Stop();
	//��ֹͣ��Ƶ//�����ļ�����ע��ڶ�������������
	hr = m_pCapture->SetOutputFileName( &MEDIASUBTYPE_Avi,inFileName, &pMux, NULL );
	//��Ⱦý�壬���������˲���
	hr = m_pCapture->RenderStream( &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pBF, NULL, pMux );
	pMux->Release();
	m_pMC->Run();//�ظ���Ƶ
	return hr;
}

void CCaptureVideo::Play()
{
	if (!m_isPlay && m_pMC)
	{
		m_pMC->Run();
		m_isPlay = true;
	}
}

void CCaptureVideo::StopPlay()
{
	if (/*m_isPlay && */m_pMC)
	{
		OAFilterState pfs;
		m_pMC->GetState(100, &pfs);

		if (pfs == 2)
		{
			m_pMC->Stop();
			m_pMC->GetState(100, &pfs);
		}
		//m_isPlay = false;
	}
}

void CCaptureVideo::SwitchVideo(BOOL bMaster)
{

}

void CCaptureVideo::SetDeviceId(int nDevId)
{
	m_ndevid = nDevId;
}

int CCaptureVideo::EnumDevices()
{
	int id = 0;
	//ö����Ƶ��׽�豸
	ICreateDevEnum *pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,
		NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)return -1;
	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
	if (hr != NOERROR)return -1;
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
	{
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag); 
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				id++;

				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
	}
	return id;
}

HRESULT GetCaptureRatio(IBaseFilter* pCapFilter, ICaptureGraphBuilder2* pBuild, REFERENCE_TIME & timeRate) {
	HRESULT hr;
	//SmartPtr<IAMStreamConfig> pam;
	IAMStreamConfig* pam;

	hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
		pCapFilter, IID_IAMStreamConfig, reinterpret_cast<void**>(&pam)); // �õ�ý����ƽӿ�

	if (hr != S_OK)
	{
		return hr;
	}

	int nCount = 0;
	int nSize = 0;
	hr = pam->GetNumberOfCapabilities(&nCount, &nSize);
	if (hr != S_OK)
	{
		return hr;
	}

	//ofstream outfile("captureframe.txt"); // ׼��д���ļ�
	// �ж��Ƿ�Ϊ��Ƶ��Ϣ
	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == nSize) {
		for (int i=0; i<nCount; i++) {
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmmt;

			hr = pam->GetStreamCaps(i, &pmmt, reinterpret_cast<BYTE*>(&scc));
			if (hr != S_OK)
			{
				return hr;
			}

			if (pmmt->formattype == FORMAT_VideoInfo) {
				VIDEOINFOHEADER* pvih = reinterpret_cast<VIDEOINFOHEADER*>(pmmt->pbFormat);
				timeRate = pvih->AvgTimePerFrame; // �õ��ɼ���֡��
				break;
			}
		}
	}

	//outfile.close(); // �ر��ļ�

	return(hr);
}

bool CCaptureVideo::GetSampleRate(int & lrate)
{
	REFERENCE_TIME timeRate;
	if (GetCaptureRatio(m_pBF, m_pCapture, timeRate) == S_OK)
	{
		lrate = 10000000 / timeRate;
		return true;
	}
	return false;
}

HRESULT CCaptureVideo::Init(int iDeviceID)
{
	HRESULT hr;
	hr = InitCaptureGraphBuilder();
	if (FAILED(hr))
	{
		long nhr = (long)hr;
		return hr;
	}
	// Bind Device Filter. We know the device because the id was passed in
	if(!BindFilter(iDeviceID, &m_pBF))
		return S_FALSE;
	hr = m_pGB->AddFilter(m_pBF, L"Capture Filter");
	// hr = m_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, 
	// m_pBF, NULL, NULL);
	// create a sample grabber

//	hr = m_pGrabber.CoCreateInstance( CLSID_SampleGrabber );
	hr = CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_ISampleGrabber, (void**)&m_pGrabber );
	if( !m_pGrabber )
	{
		long nhr = (long)hr;
		return hr;
	}

	CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabBase( m_pGrabber );

	//������Ƶ��ʽ
	AM_MEDIA_TYPE mt; 
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB32;
	
	hr = m_pGrabber->SetMediaType(&mt);
	if( FAILED( hr ) )
	{
		long nhr = (long)hr;
		return hr;
	}
	hr = m_pGB->AddFilter( pGrabBase, L"Grabber" );
	if( FAILED( hr ) )
	{
		long nhr = (long)hr;
		return hr;
	}
	// try to render preview/capture pin
	hr = m_pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,m_pBF,pGrabBase,NULL);
	if( FAILED( hr ) )
		hr = m_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,m_pBF,pGrabBase,NULL);
	if( FAILED( hr ) )
	{
		long nhr = (long)hr;
		return hr;
	}
	hr = m_pGrabber->GetConnectedMediaType( &mt );
	if ( FAILED( hr) )
	{
		long nhr = (long)hr;
		return hr;
	}

	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)mt.pbFormat;
	m_pSampleGrabberCB->lWidth = vih->bmiHeader.biWidth;
	m_pSampleGrabberCB->lHeight = vih->bmiHeader.biHeight;
	m_pSampleGrabberCB->lBitCnt = vih->bmiHeader.biBitCount;
	m_bmiHeader = vih->bmiHeader;
	
	//test
	//int nBitrate = vih->dwBitRate;
	LONGLONG nframerate = vih->AvgTimePerFrame;

	//GetCaptureRatio(m_pBF,m_pCapture);

	FreeMediaType(mt);
	hr = m_pGrabber->SetBufferSamples( FALSE );
	hr = m_pGrabber->SetOneShot( FALSE );
	hr = m_pGrabber->SetCallback(m_pSampleGrabberCB, 1);
	// 
	m_ndevid = iDeviceID;

	SetupVideoWindow();


	return S_OK;
}

void CCaptureVideo::GetVideoInfoHeader(BITMAPINFOHEADER &bmiHeader)
{
	bmiHeader = m_bmiHeader;
}

HRESULT CCaptureVideo::Preview()
{
	OAFilterState pfs;
	m_pMC->GetState(100, &pfs);
	
	HRESULT hr = m_pMC->Run();//��ʼ��Ƶ��׽
	m_pMC->GetState(100, &pfs);

	if(FAILED(hr) || pfs != 2)
	{
		long nhr = (long)hr;

		hr = ReInit();
		m_pMC->Run();//��ʼ��Ƶ��׽
		m_pMC->GetState(100, &pfs);
		if (FAILED(hr) || pfs != 2)
		{
			return hr;
		}
	}

	m_isSetupVideoWindow = true;
	m_isPlay = true;
	return S_OK;
}

bool CCaptureVideo::BindFilter(int deviceId, IBaseFilter **pFilter)
{
	if (deviceId < 0)
		return false;
	// enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)
	{
		return false;
	}
	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEm, 0);
	if (hr != NOERROR) 
	{
		return false;
	}
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	int index = 0;
	while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK, index <= deviceId)
	{
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if(SUCCEEDED(hr)) 
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) 
			{
				if (index ==deviceId)
				{
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
	}
	return true;
}

HRESULT CCaptureVideo::InitCaptureGraphBuilder()
{
	HRESULT hr;
	// ����IGraphBuilder�ӿ�
	hr=CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&m_pGB);
	// ����ICaptureGraphBuilder2�ӿ�
	hr = CoCreateInstance (CLSID_CaptureGraphBuilder2,NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **) &m_pCapture);
	if (FAILED(hr))
	{
		return hr;
	}
	m_pCapture->SetFiltergraph(m_pGB);
	hr = m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGB->QueryInterface(IID_IVideoWindow, (LPVOID *)&m_pVW);
	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
}

HRESULT CCaptureVideo::SetupVideoWindow()
{
	HRESULT hr = 0;
	hr = m_pVW->put_Owner((OAHWND)m_hWnd);
	if (FAILED(hr))
		return hr;

	hr = m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr))
		return hr;

	ResizeVideoWindow();
	if (m_hWnd == NULL)
		return m_pVW->put_AutoShow(OAFALSE);

	return hr;
}

void CCaptureVideo::ResizeVideoWindow()
{
	if (m_pVW)
	{
		//��ͼ�������������
		if (m_hWnd != NULL)
		{
			RECT rc;
			::GetClientRect(m_hWnd,&rc);
	
			int nx = (rc.right - m_pSampleGrabberCB->lWidth) / 2;
			int ny = (rc.bottom - m_pSampleGrabberCB->lHeight) / 2; 
	
			m_pVW->SetWindowPosition(nx, ny, m_pSampleGrabberCB->lWidth, m_pSampleGrabberCB->lHeight);
		} 
		else
			m_pVW->SetWindowPosition(0,0,0,0);
	} 
}

void CCaptureVideo::GrabOneFrame(BOOL bGrab)
{
	bOneShot=bGrab;
}

void CCaptureVideo::GrabOneFrameToFile(BOOL bGrab, LPCTSTR strFile)
{
	_tcscpy(szFileName, strFile);
	bOneShot=bGrab;
}

void FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0) 
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		// Strictly unnecessary but tidier
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) 
	{
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}