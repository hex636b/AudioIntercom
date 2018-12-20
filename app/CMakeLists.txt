cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/native/)

include_directories(src/main/ffmpeg/include/)
link_directories(src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/)

find_library(log-lib   log)
find_library(opensles-lib OpenSLES)

add_library(
        intercom
        SHARED
        src/main/native/intercom.c)
target_link_libraries(
        intercom
         ${opensles-lib} ${log-lib} ffmpeg)
