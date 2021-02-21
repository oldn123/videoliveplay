#include "CCamCap.h"


CCamCap::CCamCap()
{
    m_pCapVideo = Create_VideoCapturer();
    m_pCapAudio = Create_AudioCapturer();
}

CCamCap::~CCamCap()
{
    Release_VideoCapturer(m_pCapVideo);
    Release_AudioCapturer(m_pCapAudio);
}

int CCamCap::OnRealDataCallback(int nDevId, unsigned char* pBuffer, int nBufSize,
    RealDataStreamType realDataType, void* realDataInfo) {

    if (realDataType == REALDATA_VIDEO)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_pVideoBuf.reset(new uint8_t[nBufSize]);
        memcpy(m_pVideoBuf.get(), pBuffer, nBufSize);
        m_nVideoBufLen = nBufSize;
    }
    return 0;
}


int WINAPI _RealDataCallback(int nDevId, unsigned char* pBuffer, int nBufSize,
    RealDataStreamType realDataType, void* realDataInfo, void* pMaster)
{
    CCamCap* pcc = (CCamCap*)pMaster;
    return pcc->OnRealDataCallback(nDevId, pBuffer, nBufSize, realDataType, realDataInfo);
}

int CCamCap::init(int displayIndex /*= 0*/)
{
    int32_t nVideoId = 0;
    int32_t nRet = 0;
    do
    {
        if (nVideoId < 0)
        {
            nRet = -1;
            break;
        }

        HWND hWnd = nullptr;
        // 2.设置视频获取显示参数
        m_pCapVideo->SetVideoCaptureData(0, nVideoId, hWnd,
            25, 0, 0, L"RGB32", 3, 1, 1, true);

        m_pCapVideo->SetDShowCaptureCallback(_RealDataCallback, this);
        // 3.创建获取视频的图像
        nRet = m_pCapVideo->CreateCaptureGraph();
        if (nRet <= 0)
        {
            m_pCapVideo->SetCaptureVideoErr(nRet);
            break;
        }
        nRet = m_pCapVideo->BulidPrivewGraph(m_iWidth, m_iHeight, m_iBits);
        if (nRet < 0)
        {
            break;
        }
        else
        {
            if (nRet == 2)
            {
                // "Video[%d]--该设备不支持内部回显，将采用外部回显模式！(可能是因为没有可以进行绘制的表面)--In StartDSCapture()."
            }
        }

    } while (false);

    if (nRet < 0)
    {
        Release_VideoCapturer(m_pCapVideo);
        m_pCapVideo = NULL;
    }
    else
    {
        m_bCapturing = true;
    }

    return nRet;
}

int CCamCap::start()
{
    m_bCapturing = true;
    return 0;
}

int CCamCap::stop()
{
    m_bCapturing = false;
    return 0;
}

int CCamCap::captureFrame(std::shared_ptr<uint8_t>& bgraPtr, uint32_t& size)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    bgraPtr = m_pVideoBuf;
    size = m_nVideoBufLen;
    return 0;
}
