#ifndef PREFIX_H
#define PREFIX_H

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
#include "src/H264Decompress.h"

#define INBUF_SIZE 2048
using namespace std;
using namespace cv;
#endif
