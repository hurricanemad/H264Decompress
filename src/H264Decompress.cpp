#include "H264Decompress.h"

void H264Decompress::ConvertFrame(){
    cv::Mat matRGBFrame;
    //matCameraFrame = Mat(3*nMatHeight/2, nMatWidth, CV_8UC1, Scalar(0.0));

    int nLineSize = m_pavFrame->linesize[0];
    int nFrameWidth = m_pavFrame->width;
    int nFrameHeight = m_pavFrame->height;
    
    cout << "nLineSize" << nLineSize <<endl;
    cout << "nFrameWidth" << nFrameWidth <<endl;
    cout << "nFrameHeight" << nFrameHeight <<endl;
    
    int r, c;
    uchar* pmatCameraFrame;
    uchar* pmatavFrame0 = (uchar*)(m_pavFrame->data[0]);
    for(r=0; r< m_nMatHeight; r++){
        pmatCameraFrame = m_matSrcImg.ptr<uchar>(r);
        for(c=0; c < m_matSrcImg.cols; c++){
            pmatCameraFrame[c] = pmatavFrame0[r * m_pavFrame->linesize[0] + c];
        }
    }
    
    uchar* pmatavFrame1 = (uchar*)(m_pavFrame->data[1]);
    int nTempR, nTempC, ntempc;
    int n;
    for (r = 0; r < m_nMatHeight/4; r++) {
        nTempR = 2*r;
        pmatCameraFrame = m_matSrcImg.ptr<uchar>(r + m_nMatHeight);
        for(n=0; n < 2; n++){
            nTempC = n * m_matSrcImg.cols/2;
            for (c = 0; c < m_pavFrame->linesize[1]; c++) {
                ntempc = c + nTempC;
                if(c < m_matSrcImg.cols/2){
                    pmatCameraFrame[ntempc] = pmatavFrame1[r * m_pavFrame->linesize[1] + c];
                }
            }
        }
    }
    uchar* pmatavFrame2 = (uchar*)(m_pavFrame->data[2]);
    for (r = 0; r < m_nMatHeight/4; r++) {
        pmatCameraFrame = m_matSrcImg.ptr<uchar>(r + 5 * m_nMatHeight/4);
        for(n=0; n < 2; n++){
            nTempC = n * m_matSrcImg.cols/2;
            for (c = 0; c < m_pavFrame->linesize[2]; c++) {
                ntempc = c + nTempC;
                if(c < m_matSrcImg.cols/2){
                    pmatCameraFrame[ntempc] = pmatavFrame2[r * m_pavFrame->linesize[2] + c];
                }
            }
        }
    }
    cv::cvtColor(m_matSrcImg, matRGBFrame, cv::COLOR_YUV2RGB_YV12);
    cv::namedWindow("matRGBFrame");
    cv::imshow("matRGBFrame", matRGBFrame);
    cv::waitKey(30);
}

void H264Decompress::decode(){

    int nResult;
    nResult = avcodec_send_packet(m_pavCodecContext, m_pavPkt);
    if(nResult < 0){
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(-1);
    }
    
    while(nResult >= 0){
        nResult = avcodec_receive_frame(m_pavCodecContext, m_pavFrame);
        
        if(nResult == AVERROR(EAGAIN) || nResult == AVERROR_EOF)
        {
            fprintf(stderr, "Error receive frame:%d\n", nResult);
            return;
        }
        else if(nResult < 0){
            fprintf(stderr, "Error during decoding\n");
            exit(-1);
        }
        
        printf("saving frame %3d\n", m_pavCodecContext->frame_number);
        fflush(stdout);
        nResult = av_frame_make_writable(m_pavFrame);
        if (nResult < 0)
            exit(1);
        ConvertFrame();
        //snprintf(cBuf, sizeof(cBuf), "%s-%d", cFileName, pavccDecodeCtx->frame_number);
        //pgm_save(pavFrame->data[0], frame->lineszie[0], frame->width, frame->height, cBuf);
    }
}

void H264Decompress::Initialize(const char * cFileName, int nMatWidth, int nMatHeight){
    avcodec_register_all();
    m_nMatWidth = nMatWidth;
    m_nMatHeight = nMatHeight;
    m_matSrcImg = cv::Mat(3*m_nMatHeight/2, m_nMatWidth, CV_8UC1, cv::Scalar(0.0));
     
    
    //m_uinbuf = new uint8_t[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    
    memset(m_uinbuf+INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    
    m_pavCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    
    if(!m_pavCodec){
        fprintf(stderr,"Codec not found\n");
        exit(-1);
    }
    m_pavCodecParserC = av_parser_init(m_pavCodec->id);
    if(!m_pavCodecParserC){
        fprintf(stderr, "Parser not found\n");
        exit(-1);
    }
    
    m_pavCodecContext = avcodec_alloc_context3(m_pavCodec);
    
    /* put sample parameters */
    m_pavCodecContext->bit_rate = 400000;
    /* resolution must be a multiple of two */
    m_pavCodecContext->width = nMatWidth;
    m_pavCodecContext->height = nMatHeight;
    /* frames per second */
    m_pavCodecContext->time_base = (AVRational){1, 25};
    m_pavCodecContext->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    m_pavCodecContext->gop_size = 10;
    m_pavCodecContext->max_b_frames = 1;
    m_pavCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (m_pavCodec->id == AV_CODEC_ID_H264)
        av_opt_set(m_pavCodecContext->priv_data, "preset", "slow", 0);
    
    if(!m_pavCodecContext){
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(-1);
    }
    
    if(avcodec_open2(m_pavCodecContext, m_pavCodec, NULL) < 0){
        fprintf(stderr, "Could not open codec\n");
        exit(-1);
    }
    
    m_pfDecodeFile = fopen(cFileName, "rb+");
    cout << cFileName << endl;
    if(!m_pfDecodeFile){
        fprintf(stderr, "Could not open %s\n", cFileName);
        exit(-1);
    }
    
    m_pavPkt = av_packet_alloc();
    
    if(!m_pavPkt){
        cerr << "Packet alloc error!" <<endl;
        exit(-1);
    }
    
    m_pavFrame = av_frame_alloc();
    if(!m_pavFrame){
        fprintf(stderr, "Could not allocate video frame\n");
        exit(-1);
    }
    
    cout << "Initialize end!" << endl;
    /*while(!feof(m_pfDecodeFile)){
        
    m_sztDataSize = fread(m_uinbuf, 1, INBUF_SIZE, m_pfDecodeFile);
    if(!m_sztDataSize)
        break;
    cout <<  "m_sztDataSize" << m_sztDataSize <<endl;
    
    uint8_t * uData = m_uinbuf;
    while(m_sztDataSize>0){
            nResult = av_parser_parse2(m_pavCodecParserC, m_pavCodecContext, &m_pavPkt->data, &m_pavPkt->size,
                uData, m_sztDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
                
            if(nResult<0){
                fprintf(stderr, "Error while parsing\n");
                exit(-1);
            }
            uData+=nResult;
            m_sztDataSize-=nResult;
            if(m_pavPkt->size)
                decode();
        }
        
    }*/
    
}

H264Decompress::H264Decompress(const char* cFileName, int nMatWidth, int nMatHeight){
    cout << "H264Decompress initialize begin!" <<endl;
    Initialize(cFileName, nMatWidth, nMatHeight);
    cout << "H264Decompress initialize end!" <<endl;
}

void H264Decompress::Decompress(){

    
    while(!feof(m_pfDecodeFile)){
        cout << "fread" <<endl;
        m_sztDataSize = fread(m_uinbuf, 1, INBUF_SIZE, m_pfDecodeFile);
        if(!m_sztDataSize)
            break;
        
        uint8_t * uData = m_uinbuf;
        cout << "av_parser_parse2" <<endl;
        while(m_sztDataSize>0){
            nResult = av_parser_parse2(m_pavCodecParserC, m_pavCodecContext, &m_pavPkt->data, &m_pavPkt->size,
                uData, m_sztDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            
            if(nResult<0){
                fprintf(stderr, "Error while parsing\n");
                exit(-1);
            }
            uData+=nResult;
            m_sztDataSize-=nResult;
            if(m_pavPkt->size)
                decode();
        }
    }
}

H264Decompress::~H264Decompress(){
    fclose(m_pfDecodeFile);
    
    //delete m_uinbuf;
    av_parser_close(m_pavCodecParserC);
    avcodec_free_context(&m_pavCodecContext);
    av_frame_free(&m_pavFrame);
    av_packet_free(&m_pavPkt);   
}
