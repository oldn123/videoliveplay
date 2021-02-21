// dllmain.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// 如果使用 lpReserved，请将此移除
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{	
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	return 1;   // 确定
}
