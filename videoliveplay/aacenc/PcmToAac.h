#ifndef MUXER_PCMTOAAC_H
#define MUXER_PCMTOAAC_H

#include <faac.h>
#include <iostream>
#include "PcmBuffer.h"

/* *
 * @program: Muxer
 *
 * @description: 音频PCM编码为AAC格式
 *
 * @author: chenxiang
 *
 * @create: 2018-06-11 14:59
***/

typedef bool Bool;

struct SPcm2AacParam
{
    Int32 nSample = 8000;
    Int32 nChannel = 1;
    Int32 nBit = 16;

    friend std::ostream &operator<<(std::ostream &stream, const SPcm2AacParam &model)
    {
        stream << "{" << std::endl;
        stream << "    nSample       = " << model.nSample << std::endl;
        stream << "    nChannel      = " << model.nChannel << std::endl;
        stream << "    nBit          = " << model.nBit << std::endl;
        stream << "}";
        return stream;
    }
};

class PcmToAac
{
public:
    PcmToAac();

    virtual ~PcmToAac();

public:
    Bool Initilize(const SPcm2AacParam& pcm2AacParam);

    Int32 Encode(UInt8 *const pInBuf, Int32 const nInBufSize, UInt8 *const pOutBuf, Int32 const nOutBufMaxSize);

private:
    const UInt32 GetPcmBufferSize();

private:
    SPcm2AacParam m_Init;
    unsigned long m_nInputSamples = 0;//编码输入样本数
    unsigned long m_nMaxOutputBytes = 0;//最大编码输出字节数
    faacEncHandle m_hEncoder = nullptr;
    UInt8 *m_pPCMBuffer = nullptr;
    UInt8 *m_pAACBuffer = nullptr;

    PcmBuffer m_pcmBuffer; //因为编码AAC数据，需要达到规定的数据长度。保存pcm数据，满足规定大小才可取出
};


#endif //MUXER_PCMTOAAC_H
