#include "CCamCap2.h"

CCamCap2::CCamCap2()
{
}

CCamCap2::~CCamCap2()
{
}


void _CaptureDecodeCallbackFunc_t(LPVOID pUser, BYTE* pBuffer, long lBufferSize, long lWidth, long lHeight, long lBitCnt)
{
    CCamCap2* pCap = (CCamCap2*)pUser;

    pCap->OnRealDataCallback(pBuffer, lBufferSize, 0, 0);

    return;
}


int CCamCap2::OnRealDataCallback(unsigned char* pBuffer, int nBufSize,
    int realDataType, void* realDataInfo) {

    if (realDataType == 0)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_pVideoBuf.reset(new uint8_t[nBufSize]);
        memcpy(m_pVideoBuf.get(), pBuffer, nBufSize);
        m_nVideoBufLen = nBufSize;
    }
    return 0;
}


int CCamCap2::init(int displayIndex /*= 0*/)
{
    int n = CameraCatchMgr::Instance().GetCameraCount();
    if (n > 0)
    {
        m_pCap = CameraCatchMgr::Instance().GetCaptureByIndex(0);
        if (m_pCap)
        {
            m_pCap->Init(0);

            m_pCap->SetCallbackFunc(_CaptureDecodeCallbackFunc_t, this);

            m_pCap->GetVideoInfoHeader(m_bmpInfo);

           // m_pCap->Preview();
        }
    }

    return 0;
}

int CCamCap2::start()
{
    m_bCapturing = true;
    m_pCap->Play();
    return 0;
}

int CCamCap2::stop()
{
    m_bCapturing = false;
    m_pCap->StopPlay();
    return 0;
}

int CCamCap2::captureFrame(std::shared_ptr<uint8_t>& bgraPtr, uint32_t& size)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    bgraPtr = m_pVideoBuf;
    size = m_nVideoBufLen;
    return bgraPtr && size > 0 ? 0 : 1;
}










/****************************************************/




CAudioCap::CAudioCap() {
    m_pAud = new CCaptureAudio;
}

CAudioCap::~CAudioCap() {
    delete m_pAud;
}

int CAudioCap::readSamples(uint8_t* data, uint32_t samples)
{
    return m_pAud->GetCurrentBuffer(data, samples);
}

int CAudioCap::getSamples() {
    return m_pAud->getSamples();
}

int CAudioCap::getSamplerate() {
    return m_pAud->getSamplerate();
}

int CAudioCap::getChannels() {
    return m_pAud->getChannels();
}

int CAudioCap::getBitsPerSample()
{
    return m_pAud->getBitsPerSample();
}

int CAudioCap::init()
{
    return m_pAud->Init();
}

int CAudioCap::start() {

    m_pAud->BulidCaptureGraph();
    m_bCapturing = true;
    return 0;
}

int CAudioCap::stop() {
    return m_pAud->stop();
}