cmake_minimum_required(VERSION 3.4.1)

add_library( # Sets the name of the library.
             intercom
             SHARED
        src/main/intercom/intercom.c)

include_directories(src/main/intercom/)

add_library( ffmpeg
        SHARED
        IMPORTED )
set_target_properties( ffmpeg
        PROPERTIES IMPORTED_LOCATION
        #./src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libffmpeg.so )
        ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libffmpeg.so )
include_directories(src/main/ffmpeg/include/)


find_library(log-lib   log)
find_library(opensles-lib OpenSLES)

target_link_libraries(
        intercom
        ffmpeg ${opensles-lib} ${log-lib} )
