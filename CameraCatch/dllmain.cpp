// dllmain.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// ���ʹ�� lpReserved���뽫���Ƴ�
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{	
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	return 1;   // ȷ��
}
