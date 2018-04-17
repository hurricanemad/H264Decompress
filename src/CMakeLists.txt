cmake_minimum_required(VERSION 2.6)

find_package(OpenCV REQUIRED)
include_directories(/usr/local/include)
include_directories( ${OPENCV_INCLUDE_DIR} )
link_directories(/usr/local/lib)

add_library(FrameDecompress  SHARED  H264Decompress.cpp)

target_link_libraries(FrameDecompress avcodec avdevice avfilter avformat avutil postproc swresample swscale x264)
target_link_libraries(FrameDecompress ${OpenCV_LIBS})

install(TARGETS FrameDecompress LIBRARY DESTINATION install/lib)
