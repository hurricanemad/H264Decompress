#ifndef H264_DECOMPRESS_H
#define H264_DECOMPRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
using namespace std;
#define INBUF_SIZE 2048

class H264Decompress{    
public:
    H264Decompress(){};
    H264Decompress(const char* /*cFileName*/, int /*nMatWidth*/, int /*nMatHeight*/);
    ~H264Decompress();

    void ConvertFrame();
    void Decompress();

private:
    void Initialize(const char */*cFileName*/, int /*nMatWidth*/, int /*nMatHeight*/);

    void decode();
    const char *m_pcFileName;
    const AVCodec *m_pavCodec;
    AVCodecParserContext *m_pavCodecParserC;
    AVCodecContext *m_pavCodecContext;
    FILE* m_pfDecodeFile;
    AVFrame *m_pavFrame;
    uint8_t m_uinbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *m_uData;
    size_t m_sztDataSize;
    int nResult;
    AVPacket *m_pavPkt;
    int m_nMatWidth;
    int m_nMatHeight;
    cv::Mat m_matSrcImg;
};

#endif
