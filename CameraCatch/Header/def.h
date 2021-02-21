#pragma once


#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x )if ( NULL != x ){x->Release();x = NULL;}
#endif

#ifdef _Export
#define VisCameraCatchExport __declspec(dllexport)
#else
#define VisCameraCatchExport __declspec(dllexport)
#endif // DEBUG



typedef void (*CaptureDecodeCallbackFunc_t)(LPVOID pUser, BYTE* pBuffer, long lBufferSize, long lWidth, long lHeight, long lBitCnt);


