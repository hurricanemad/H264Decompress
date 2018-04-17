cmake_minimum_required(VERSION 2.6)
project(cameradecode)

find_package(OpenCV REQUIRED)
include_directories(/usr/local/include)
include_directories( ${OPENCV_INCLUDE_DIR} )
link_directories(/usr/local/lib)
link_directories(./lib)
add_subdirectory(src src)

add_executable(cameradecode main.cpp)

target_link_libraries(cameradecode avcodec avdevice avfilter avformat avutil postproc swresample swscale x264)
target_link_libraries(cameradecode ${OpenCV_LIBS})
target_link_libraries(cameradecode FrameDecompress)

install(TARGETS cameradecode RUNTIME DESTINATION install)
