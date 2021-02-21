#include "StdAfx.h"
#include "../Header/CameraCatchMgr.h"
#include "../Header/CaptureVideo.h"

CameraCatchMgr* CameraCatchMgr::s_pInstancePtr = NULL;

CameraCatchMgr& CameraCatchMgr::Instance()
{
	if (s_pInstancePtr == NULL)
		s_pInstancePtr = new CameraCatchMgr;
	return *s_pInstancePtr;
}

void CameraCatchMgr::DeleteInstance()
{
	if (s_pInstancePtr)
	{
		delete s_pInstancePtr;
		s_pInstancePtr = NULL;
	}
}

CameraCatchMgr::CameraCatchMgr(void)
{
	m_cameraCount = CCaptureVideo::EnumDevice();
}

CameraCatchMgr::~CameraCatchMgr(void)
{
	for (int idx = 0; idx < m_mapCameras.size(); ++idx)
	{
		CCaptureVideo *p = m_mapCameras[idx];
		if (p != NULL)
		{
			p->Release();
			delete p;
		}
	}
	m_mapCameras.clear();
}

int CameraCatchMgr::GetCameraCount()
{
	return m_cameraCount;
}

CCaptureVideo* CameraCatchMgr::GetCaptureByIndex(int index)
{
	auto it = m_mapCameras.find(index);
	if (it != m_mapCameras.end())
	{
		CCaptureVideo* pcap = it->second;
		return pcap;
		//if (pcap->ReInit() == S_OK)
		//{
		//	WriteLogEx(L"Call CCaptureVideo ReInit camera id:%d OK!", index);
		//	return pcap;
		//}

		//m_mapCameras.erase(it);
		//WriteLogEx(L"Call CCaptureVideo ReInit camera id:%d Failed!", index);
		//return NULL;
	}

	CCaptureVideo *pcap = new CCaptureVideo;
	if (pcap->Init(index) == S_OK)
	{
		m_mapCameras[index] = pcap;
		return pcap;
	}
	else
	{
		delete pcap;
		return NULL;
	}
}
