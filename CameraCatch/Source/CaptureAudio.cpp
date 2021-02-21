#include "stdafx.h"

#include "..\header\CaptureAudio.h"
#include <strmif.h>
#include <atlcomcli.h>
#include <mtype.h>
#include <assert.h>
#include "../header/cb.hpp"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void _CaptureDecodeCallbackFunc(LPVOID pUser, BYTE* pBuffer, long lBufferSize, long lWidth, long lHeight, long lBitCnt) {
	CCaptureAudio* pThis = (CCaptureAudio*)pUser;
	pThis->OnBufferCallback(pBuffer, lBufferSize);
}


FILE* g_fp = nullptr;

CCaptureAudio::CCaptureAudio()
{
	m_pGraphBuilder = NULL;
	
	m_pCaptureGraphBulid = NULL;
	m_pBaseFilter = NULL;
	m_pVideoWin = NULL; 
	m_pMediaCon = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pSampleGrabber =NULL;

	m_nChannels=2;
	m_nBytesPerSample=16;//位数16位
	m_nSampleRate=44100;//44100;,采样率
	m_nAudioBufferType=0;
	m_nPinType=1;
	m_nBufferSize=2;

	m_pSampleGrabberCB = new CSampleGrabberCB;
	m_pSampleGrabberCB->m_fn = _CaptureDecodeCallbackFunc;
	m_pSampleGrabberCB->m_cxt = this;
    m_pSampleGrabberCB->lWidth= 0;
    m_pSampleGrabberCB->lHeight= 0;
    m_pSampleGrabberCB->lBitCnt= 0;

	g_fp = fopen("wav.pcm", "wb+");
}

CCaptureAudio::~CCaptureAudio()
{
	if(m_threadptr)
	{
		m_threadptr->join();
	}

	if(m_pMediaCon)
	{
		m_pMediaCon->Stop();
		m_pMediaCon->Release();
		m_pMediaCon = NULL;
	}
	if(m_pVideoWin)
	{
		m_pVideoWin->put_Visible(OAFALSE);
		m_pVideoWin->put_Owner(NULL);
		m_pVideoWin->Release();
		m_pVideoWin = NULL;
	}
	TearDownGraph();
	SAFE_RELEASE(m_pGraphBuilder);
	SAFE_RELEASE(m_pCaptureGraphBulid);
	SAFE_RELEASE(m_pSampleGrabberFilter);
	SAFE_RELEASE(m_pSampleGrabber);
	m_nBufferSize=0;
	m_spDataBuffer = nullptr;
	if (m_pSampleGrabberCB)
		delete m_pSampleGrabberCB;
//	CoUninitialize();
}

void CCaptureAudio::OnBufferCallback(uint8_t* pbuf, int32_t nlen) {
	
	std::vector<uint8_t> arr;
	arr.resize(nlen);
	memcpy(arr.data(), pbuf, nlen);

	if (g_fp)
	{
		fwrite(pbuf, 1, nlen, g_fp);
		fflush(g_fp);
	}

	std::lock_guard lk(m_muxData);
	m_dataList.emplace_back(arr);
	m_dataSize += nlen;
}


HRESULT GetPin(IBaseFilter* pFilter, PIN_DIRECTION dirrequired, int iNum, IPin** ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

    if (!pFilter)
        return E_POINTER;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr))
        return hr;

    ULONG ulFound;
    IPin* pPin;
    hr = E_FAIL;

    while (S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        pPin->QueryDirection(&pindir);
        if (pindir == dirrequired)
        {
            if (iNum == 0)
            {
                *ppPin = pPin;  // Return the pin's interface
                hr = S_OK;      // Found requested pin, so clear error
                break;
            }
            iNum--;
        }

        pPin->Release();
    }

    return hr;
}


//断掉Graph
void CCaptureAudio::TearDownGraph()
{
	//先释放掉没有使用的
	if(m_pVideoWin)
	{
		m_pVideoWin->put_Owner(NULL);
		m_pVideoWin->put_Visible(OAFALSE);
		m_pVideoWin->Release();
		m_pVideoWin = NULL;
	}
	if(m_pBaseFilter)
        NukeDownstream(m_pGraphBuilder,m_pBaseFilter);
   	SAFE_RELEASE(m_pGraphBuilder);
	SAFE_RELEASE(m_pCaptureGraphBulid);
	SAFE_RELEASE(m_pSampleGrabberFilter);
	SAFE_RELEASE(m_pSampleGrabber);
}
// Tear down everything downstream of a given filter
//关闭下游所有的连接
void CCaptureAudio::NukeDownstream(IGraphBuilder * inGraph, IBaseFilter * inFilter) 
{
	if (inGraph && inFilter)
	{
		IEnumPins * pinEnum = 0;
		if (SUCCEEDED(inFilter->EnumPins(&pinEnum)))
		{
			pinEnum->Reset();
			IPin * pin = 0;
			ULONG cFetched = 0;
			bool pass = true;
			while (pass && SUCCEEDED(pinEnum->Next(1, &pin, &cFetched)))
			{
				if (pin && cFetched)
				{
					IPin * connectedPin = 0;
					pin->ConnectedTo(&connectedPin);
					if(connectedPin) 
					{
						PIN_INFO pininfo;
						if (SUCCEEDED(connectedPin->QueryPinInfo(&pininfo)))
						{
							if(pininfo.dir == PINDIR_INPUT) 
							{
								NukeDownstream(inGraph, pininfo.pFilter);
								inGraph->Disconnect(connectedPin);
								inGraph->Disconnect(pin);
								inGraph->RemoveFilter(pininfo.pFilter);
							}
							pininfo.pFilter->Release();
						}
						connectedPin->Release();
					}
					pin->Release();
				}
				else
				{
					pass = false;
				}
			}
			pinEnum->Release();
		}
	}
}


//创建视频捕获Graph
HRESULT CCaptureAudio::CreateCaptureGraphBuilder()
{
	HRESULT hr=NOERROR;
	
	if(m_pGraphBuilder==NULL)
	{
		hr=CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&m_pGraphBuilder);
		if(FAILED(hr))
		{
			//ERR_DEBUG("CreateCaptureGraphBuilder Create m_pGraphBuilder Failed");
			return hr;
		}	
	}
	if(m_pCaptureGraphBulid==NULL)
	{
		hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
			IID_ICaptureGraphBuilder2, (void **) &m_pCaptureGraphBulid);
		if (FAILED(hr))		
		{
			//ERR_DEBUG("CreateCaptureGraphBuilder CaptureGraphBuilder2 Failed");
			return hr;
		}	
		hr = m_pCaptureGraphBulid->SetFiltergraph(m_pGraphBuilder);
	}
	if(m_pMediaCon==NULL)
	{
		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaCon);
		if (FAILED(hr))
		{
			//ERR_DEBUG("CreateCaptureGraphBuilder  QueryInterface m_pMediaCon Failed");
			return hr;
		}
	}
	return hr;
}

//连接音频设备，并将音频设备加入到
BOOL CCaptureAudio::BindToAudioDev(int deviceId, IBaseFilter **pFilter)
{
	if (deviceId < 0)
	{
		return FALSE;
	}
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)
	{
		return FALSE;
	}
	CComPtr<IEnumMoniker> pEm;
	//此处与视频不同的地方
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,&pEm, 0);
	if (hr != NOERROR)
	{
		return FALSE;
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
				if (index == deviceId)
				{
					//将视频设备绑定到基础过滤器上
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
					//m_iDeviceId = deviceId;
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
	}
	return TRUE;
}

//初始化声卡并建立设备,传输进入一个pGraph参数
bool CCaptureAudio::Init()
{
	//创建Capture
	int bFlag = 1;
	if(FAILED(CreateCaptureGraphBuilder()))
	{
		return false;
	}
	
	HRESULT hr=NOERROR;
	hr = BindToAudioDev(m_iDeviceId,&m_pBaseFilter);
	if(FAILED(hr))
	{
		m_iDeviceId=-1;
		return false;
	}

	hr = m_pGraphBuilder->AddFilter(m_pBaseFilter, L"Capture");
	if (FAILED(hr))
		return false;

	return true;

}


HRESULT CCaptureAudio::CreateCaptureSampleGrabber()
{
	HRESULT hr=NOERROR;
	if(m_pSampleGrabberFilter==NULL)
	{
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, 
			(LPVOID *)&m_pSampleGrabberFilter);
		if(FAILED(hr))
		{
			SAFE_RELEASE(m_pSampleGrabberFilter);
			//ERR_DEBUG("CreateCaptureGraphBuilder  QueryInterface m_pSampleGrabberFilter Failed");
			return hr;
		}
	}
	if(m_pSampleGrabber==NULL)
	{
		hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);
		if(FAILED(hr))
		{
			SAFE_RELEASE(m_pSampleGrabberFilter);
			SAFE_RELEASE(m_pSampleGrabber);
			return hr;
		}

		CMediaType audioType;
		audioType.SetType(&MEDIATYPE_Audio);
		hr = m_pSampleGrabber->SetMediaType( &audioType );
		hr = m_pGraphBuilder->AddFilter(m_pSampleGrabberFilter, L"Grabber");
	}
	return hr;
}


/*
函数: BulidPrivewGraph()
功能: 创建函数
*/
int CCaptureAudio::BulidCaptureGraph()
{
	HRESULT hr=NOERROR;
	if(m_iDeviceId <0)
	{
		return -1;
	}
	// 解决声卡不存在时出现调用程序异常的问题 [10/23/2014-16:28:00 Dingshuai]
	if(m_pBaseFilter==NULL)
	{
		return -1;
	}

	GUID pCategorySuc = PIN_CATEGORY_PREVIEW;
	GUID pCategoryFail = PIN_CATEGORY_CAPTURE;
	if(m_nPinType==0)
	{
		pCategorySuc = PIN_CATEGORY_PREVIEW;
		pCategoryFail = PIN_CATEGORY_CAPTURE;

	}
	else
	{
		pCategorySuc = PIN_CATEGORY_CAPTURE;
		pCategoryFail = PIN_CATEGORY_PREVIEW;	
	}
	hr = SetAudioFormat(m_nChannels,m_nBytesPerSample,m_nSampleRate,m_nAudioBufferType,m_nPinType);
	
	if(FAILED(hr))
	{
		//ERR_DEBUG("SetAudioFormat Failed");
		return -1;
		
	}
	hr = CreateCaptureSampleGrabber();
	if(FAILED(hr))
	{
		SAFE_RELEASE(m_pSampleGrabberFilter);
		SAFE_RELEASE(m_pSampleGrabber);
		//ERR_DEBUG("CreateCaptureSampleGrabber Failed");
		return -1;
	}

	hr = m_pCaptureGraphBulid->RenderStream(&pCategorySuc,&MEDIATYPE_Audio,m_pBaseFilter,NULL,m_pSampleGrabberFilter);
	if(FAILED(hr))
	{	
			//ERR_DEBUG("PrivewVideoDev RenderStream Failed ");
			return -1;
	}

	int nMode=1;//0--SampleCB,1--BufferCB
	m_pSampleGrabber->SetCallback(m_pSampleGrabberCB, nMode);

	hr = StartPreview();
	if(FAILED(hr))
	{
		return -1;
	}
	return 1;
}

HRESULT CCaptureAudio::StartPreview()
{
	HRESULT hr=NOERROR;
	if(m_pMediaCon==NULL)
	{
		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaCon);
		if(SUCCEEDED(hr))
		{
			hr = m_pMediaCon->Run();
			if(FAILED(hr))
			{
				m_pMediaCon->Stop();
			}
		}
	}
	else
	{
		hr = m_pMediaCon->Run();
		if(FAILED(hr))
		{
			m_pMediaCon->Stop();
		}
	}
	return hr;
}

HRESULT CCaptureAudio::StopPreview()
{
	HRESULT hr=NOERROR;
	if(m_pMediaCon==NULL)
	{
		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaCon);
		if(SUCCEEDED(hr))
		{
			hr = m_pMediaCon->Stop();
		}
		if(FAILED(hr))
		{
			return hr;
		}
	}
	else
	{
		hr = m_pMediaCon->Stop();
	}
	return hr;
}

HRESULT CCaptureAudio::SetAudioFormat(int nChannels,int nBytesPerSample,long nFrequency,int nAudioBufferType,int nPinType)
{
	HRESULT   hr=0;   
    IAMBufferNegotiation   *pAMbufNeg=0;   
    IAMStreamConfig   *pAMStreamConfig=0; 
	

	if(m_pBaseFilter==NULL)
		return -1;
//	int   nChannels=2;   
//	int   nBytesPerSample=(int)2; //16位
//	long   nFrequency=22050;//44100;    

	nBytesPerSample=nBytesPerSample/8;
    long   lBytesPerSecond  = (long)   (nBytesPerSample*nFrequency*nChannels);          
    long   lBufferSize=1024*128;//(long)((float)lBytesPerSecond*0.05);//50ms 
	BOOL bFlag=TRUE;
	if(nAudioBufferType==0)
	{
		lBufferSize=1024*128;
		bFlag=TRUE;
	}
	else if(nAudioBufferType==1)
	{
		lBufferSize=(long)((float)lBytesPerSecond*0.05);
		bFlag=TRUE;
	}
	else if(nAudioBufferType==2)
	{
		bFlag=FALSE;
	}
	else if(nAudioBufferType>=512)
	{
		lBufferSize=nAudioBufferType;
		bFlag=TRUE;
	}
	if(bFlag==TRUE)
	{
		//修改捕获缓冲区的大小
		IPin   *pPin=0; 
		hr = GetPin( m_pBaseFilter, PINDIR_OUTPUT, 0, &pPin);  
		if(SUCCEEDED(hr)) 
		{
			hr=pPin->QueryInterface(IID_IAMBufferNegotiation,(void**)&pAMbufNeg);
			if(FAILED(hr))   
			{   
				return hr; 
				
			}
			ALLOCATOR_PROPERTIES   prop={0};
			//Alignment of the buffer; buffer start will be aligned on a multiple of this value
			prop.cbAlign   =   nBytesPerSample * nChannels;//   -1   means   no   preference.   
			prop.cbBuffer   =   lBufferSize;//每一个缓冲区的长度
			prop.cbPrefix   =   -1;   
			prop.cBuffers   =   8; //缓冲区的块数
			hr   =   pAMbufNeg->SuggestAllocatorProperties(&prop);   
			pAMbufNeg->Release();
			
		}
	}
	//设置音频采样等等参数
	hr = m_pCaptureGraphBulid->FindInterface(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Audio,
		m_pBaseFilter,IID_IAMStreamConfig,(void **)&pAMStreamConfig);
	AM_MEDIA_TYPE   *pmt={0};   
	hr   =   pAMStreamConfig->GetFormat(&pmt);   
	if   (SUCCEEDED(hr))   
	{   
		WAVEFORMATEX   *pWF   =   (WAVEFORMATEX   *)   pmt->pbFormat;   
		pWF->wFormatTag = WAVE_FORMAT_PCM;
		pWF->nChannels   = (WORD)   nChannels;   
		pWF->nSamplesPerSec   =nFrequency;   
		pWF->nAvgBytesPerSec =  lBytesPerSecond;   
		pWF->wBitsPerSample  =  (nBytesPerSample   *   8);   
		pWF->nBlockAlign   = (WORD)   (nBytesPerSample*2   );   
		hr = pAMStreamConfig->SetFormat(pmt);   
		FreeMediaType(*pmt);   
	}  
	pAMStreamConfig->Release();
	return   hr;   
}

int CCaptureAudio::GetCurrentBuffer(uint8_t* pbuf, uint32_t lDatasize)
{
	std::lock_guard lk(m_muxData);
	if (lDatasize > m_dataSize)
	{
		return 0;
	}

	m_dataSize -= lDatasize;

	auto nNeedLen = lDatasize;

	int32_t noffset = 0;
	while (!m_dataList.empty())
	{
		auto & item = m_dataList.front();
		if (item.size() <= nNeedLen)
		{
			nNeedLen -= item.size();
			memcpy(pbuf + noffset, item.data(), item.size());
			noffset += item.size();
			m_dataList.pop_front();
		}
		else
		{
			memcpy(pbuf + noffset, item.data(), nNeedLen);
			item.erase(item.begin(), item.begin() + nNeedLen);
			break;
		}
	} 

	return lDatasize;

// 	if(m_pSampleGrabber==NULL)
// 	{
// 		lDatasize=0;
// 		return NULL;
// 	}
// 	long  lSize = 0;
// 	HRESULT hr=NOERROR;
// 	hr = m_pSampleGrabber->GetCurrentBuffer(&lSize, NULL);
// 	if (FAILED(hr))
// 	{
// 		lDatasize=0;
// 		return NULL;
// 	}
// 
//     if (lSize <= 0)
//     {
//         lDatasize = 0;
//         return NULL;
//     }
// 
// 	assert(lSize <= lDatasize);
// 
// 	hr = m_pSampleGrabber->GetCurrentBuffer((long*)&lDatasize,(long*)pbuf);
// 	if (FAILED(hr))
// 	{
// 		lDatasize=0;
// 		return NULL;
// 	}
// 	return true;
}

int CCaptureAudio::getSamples() {
    std::lock_guard lk(m_muxData);
	return m_dataSize * 8 / m_nBytesPerSample / m_nChannels;
}

int CCaptureAudio::getSamplerate() {
	return m_nSampleRate;
}

int CCaptureAudio::getChannels() {
	return m_nChannels;
}

int CCaptureAudio::getBitsPerSample() {
	return m_nBytesPerSample;
}

int CCaptureAudio::stop() {

	StopPreview();

	return 0;
}
