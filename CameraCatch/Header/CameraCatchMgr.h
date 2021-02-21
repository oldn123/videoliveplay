#pragma once
#include <map>

#ifdef _Export
#define CameraCatchExport __declspec(dllexport)
#else
#define CameraCatchExport __declspec(dllimport)
#endif // DEBUG


class CCaptureVideo;
class CameraCatchExport CameraCatchMgr
{
public:
	static CameraCatchMgr& Instance();
	static void DeleteInstance();

	int GetCameraCount();
	CCaptureVideo* GetCaptureByIndex(int index);

protected:
	CameraCatchMgr(void);
	~CameraCatchMgr(void);

private:
	static CameraCatchMgr* s_pInstancePtr;

	int m_cameraCount;
	std::map<int, CCaptureVideo*> m_mapCameras;
};
