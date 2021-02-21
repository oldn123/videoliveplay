#ifndef MUXER_PCMTOAAC_H
#define MUXER_PCMTOAAC_H

#include <faac.h>
#include <iostream>
#include "PcmBuffer.h"

/* *
 * @program: Muxer
 *
 * @description: ��ƵPCM����ΪAAC��ʽ
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
    unsigned long m_nInputSamples = 0;//��������������
    unsigned long m_nMaxOutputBytes = 0;//����������ֽ���
    faacEncHandle m_hEncoder = nullptr;
    UInt8 *m_pPCMBuffer = nullptr;
    UInt8 *m_pAACBuffer = nullptr;

    PcmBuffer m_pcmBuffer; //��Ϊ����AAC���ݣ���Ҫ�ﵽ�涨�����ݳ��ȡ�����pcm���ݣ�����涨��С�ſ�ȡ��
};


#endif //MUXER_PCMTOAAC_H
