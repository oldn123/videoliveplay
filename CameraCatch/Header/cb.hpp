
#pragma once

#include <dshow.h>
#include <qedit.h>

#include "def.h"


class CSampleGrabberCB : public ISampleGrabberCB
{
public:
    CaptureDecodeCallbackFunc_t m_fn;
    void* m_cxt;

    long lWidth;
    long lHeight;
    long lBitCnt;
    CSampleGrabberCB() :m_fn(NULL)
    {
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return 2;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        return 1;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown)
        {
            *ppv = (void*) static_cast<ISampleGrabberCB*> (this);
            return NOERROR;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample)
    {
        return 0;
    }
    STDMETHODIMP BufferCB(double dblSampleTime, BYTE* pBuffer, long lBufferSize)
    {
        if (m_fn != NULL)
        {
            m_fn(m_cxt, pBuffer, lBufferSize, lWidth, lHeight, lBitCnt);
            return 0;
        }

        return 0;
    }
};
