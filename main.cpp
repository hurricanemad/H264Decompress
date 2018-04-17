#include"prefix.h"

void pgm_save(unsigned char *pcBuf, int nWrap, int nXSize, int nYSize, char* cFileName){
        FILE *pfile;
        int i;
        pfile = fopen(cFileName, "w");
        fprintf(pfile, "P5\n%d %d\n%d\n", nXSize, nYSize, 255);
        for(i = 0; i < nYSize;  i++)
            fwrite(pcBuf + i*nWrap, 1, nXSize, pfile);
        fclose(pfile);
}

void ConvertFrame(AVFrame* pavFrame, Mat& matCameraFrame){
    int nMatWidth = 640;
    int nMatHeight = 360;
    
    Mat matRGBFrame;
    //matCameraFrame = Mat(3*nMatHeight/2, nMatWidth, CV_8UC1, Scalar(0.0));

    int nLineSize = pavFrame->linesize[0];
    int nFrameWidth = pavFrame->width;
    int nFrameHeight = pavFrame->height;
    
    cout << "nLineSize" << nLineSize <<endl;
    cout << "nFrameWidth" << nFrameWidth <<endl;
    cout << "nFrameHeight" << nFrameHeight <<endl;
    
    int r, c;
    uchar* pmatCameraFrame;
    uchar* pmatavFrame0 = (uchar*)(pavFrame->data[0]);
    for(r=0; r< nMatHeight; r++){
        pmatCameraFrame = matCameraFrame.ptr<uchar>(r);
        for(c=0; c < matCameraFrame.cols; c++){
            pmatCameraFrame[c] = pmatavFrame0[r * pavFrame->linesize[0] + c];
        }
    }
    
    uchar* pmatavFrame1 = (uchar*)(pavFrame->data[1]);
    int nTempR, nTempC, ntempc;
    int n;
    for (r = 0; r < nMatHeight/4; r++) {
        nTempR = 2*r;
        pmatCameraFrame = matCameraFrame.ptr<uchar>(r + nMatHeight);
        for(n=0; n < 2; n++){
            nTempC = n * matCameraFrame.cols/2;
            for (c = 0; c < pavFrame->linesize[1]; c++) {
                ntempc = c + nTempC;
                if(c < matCameraFrame.cols/2){
                    pmatCameraFrame[ntempc] = pmatavFrame1[r * pavFrame->linesize[1] + c];
                }
            }
        }
    }
    uchar* pmatavFrame2 = (uchar*)(pavFrame->data[2]);
    for (r = 0; r < nMatHeight/4; r++) {
        pmatCameraFrame = matCameraFrame.ptr<uchar>(r + 5 * nMatHeight/4);
        for(n=0; n < 2; n++){
            nTempC = n * matCameraFrame.cols/2;
            for (c = 0; c < pavFrame->linesize[2]; c++) {
                ntempc = c + nTempC;
                if(c < matCameraFrame.cols/2){
                    pmatCameraFrame[ntempc] = pmatavFrame2[r * pavFrame->linesize[2] + c];
                }
            }
        }
    }
    cvtColor(matCameraFrame, matRGBFrame, cv::COLOR_YUV2RGB_YV12);
    namedWindow("matRGBFrame");
    imshow("matRGBFrame", matRGBFrame);
    waitKey(30);
}

void decode(AVCodecContext* pavccDecodeCtx, AVFrame* pavFrame, AVPacket *pavPkt){
    int nMatWidth = 640;
    int nMatHeight = 360;
    //char cBuf[1024];

    int nResult;
    nResult = avcodec_send_packet(pavccDecodeCtx, pavPkt);
    if(nResult < 0){
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(-1);
    }
    
    while(nResult >= 0){
        nResult = avcodec_receive_frame(pavccDecodeCtx, pavFrame);
        Mat matCameraFrame = Mat(3*nMatHeight/2, nMatWidth, CV_8UC1, Scalar(0.0));
        if(nResult == AVERROR(EAGAIN) || nResult == AVERROR_EOF)
            return;
        else if(nResult < 0){
            fprintf(stderr, "Error during decoding\n");
            exit(-1);
        }
        
        printf("saving frame %3d\n", pavccDecodeCtx->frame_number);
        fflush(stdout);
        
        ConvertFrame(pavFrame, matCameraFrame);
        //snprintf(cBuf, sizeof(cBuf), "%s-%d", cFileName, pavccDecodeCtx->frame_number);
        //pgm_save(pavFrame->data[0], frame->lineszie[0], frame->width, frame->height, cBuf);
    }
}

int main(int argc, char **argv) {
#if 0
    const char *cFileName, *cOutFileName;
    const AVCodec *pavCodec;
    AVCodecParserContext *pavCodecPC;
    AVCodecContext *pavCodecContext;
    FILE* pfDecode;
    AVFrame *pavFrame;
    uint8_t uinbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *uData;
    size_t sztDataSize;
    int nResult;
    AVPacket *pavPkt;
    
    cFileName = "/home/dox/Algorithm/CameraCompress/build/install/camera.h264";
    cOutFileName = "";
    
    avcodec_register_all();
    
    pavPkt = av_packet_alloc();
    
    if(!pavPkt){
        cerr << "Packet alloc error!" <<endl;
        exit(-1);
    }
    
    memset(uinbuf+INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    
    pavCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    
    if(!pavCodec){
        fprintf(stderr,"Codec not found\n");
        exit(-1);
    }
    pavCodecPC = av_parser_init(pavCodec->id);
    if(!pavCodecPC){
        fprintf(stderr, "Parser not found\n");
        exit(-1);
    }
    
    pavCodecContext = avcodec_alloc_context3(pavCodec);
    
    /* put sample parameters */
    pavCodecContext->bit_rate = 400000;
    /* resolution must be a multiple of two */
    pavCodecContext->width = 640;
    pavCodecContext->height = 360;
    /* frames per second */
    pavCodecContext->time_base = (AVRational){1, 25};
    pavCodecContext->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames, NUL
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    pavCodecContext->gop_size = 10;
    pavCodecContext->max_b_frames = 1;
    pavCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (pavCodec->id == AV_CODEC_ID_H264)
        av_opt_set(pavCodecContext->priv_data, "preset", "slow", 0);
    
    if(!pavCodecContext){
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(-1);
    }
    
    if(avcodec_open2(pavCodecContext, pavCodec, NULL) < 0){
        fprintf(stderr, "Could not open codec\n");
        exit(-1);
    }
    
    pfDecode = fopen(cFileName, "rb+");
    if(!pfDecode){
        fprintf(stderr, "Could not open %s\n", cFileName);
        exit(-1);
    }
    
    pavFrame = av_frame_alloc();
    if(!pavFrame){
        fprintf(stderr, "Could not allocate video frame\n");
        exit(-1);
    }
    
    while(!feof(pfDecode)){
        sztDataSize = fread(uinbuf, 1, INBUF_SIZE, pfDecode);
        if(!sztDataSize)
            break;
        
        uData = uinbuf;
        while(sztDataSize>0){
            nResult = av_parser_parse2(pavCodecPC, pavCodecContext, &pavPkt->data, &pavPkt->size,
                uData, sztDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            
            if(nResult<0){
                fprintf(stderr, "Error while parsing\n");
                exit(-1);
            }
            uData+=nResult;
            sztDataSize-=nResult;
            if(pavPkt->size)
                decode(pavCodecContext, pavFrame, pavPkt);
        }
    }
    decode(pavCodecContext, pavFrame, NULL);
    fclose(pfDecode);
    
    av_parser_close(pavCodecPC);
    avcodec_free_context(&pavCodecContext);
    av_frame_free(&pavFrame);
    av_packet_free(&pavPkt);  
#else
    const char * pcFileName = "/home/dox/Algorithm/CameraCompress/build/install/camera.h264";
    H264Decompress h264decompress(pcFileName, 640, 360);
    cout << "H264Decompress initialize!" <<endl;    
    h264decompress.Decompress();
    cout << "H264Decompress decode end!" <<endl;    
#endif
    
    return 0;
}
