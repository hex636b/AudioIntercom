//
// Created by konga on 2018/12/7.
//
#include <stdio.h>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <android/log.h>
#include <jni.h>
#include "libavformat/avformat.h"

#include "talkback.h"

#define TAG "talkback"

#define logd(fmt, arg...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##arg)
#define logi(fmt, arg...)  __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##arg)
#define loge(fmt, arg...)  __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##arg)


int talkback_start(JNIEnv *env, jobject jobj) {
    SLresult res;
    SLObjectItf sl;

    SLEngineOption EngineOption[] = {
            (SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE,
    };
    res = slCreateEngine(&sl, 1, EngineOption, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != res) {
        loge("create engine fail");
        return -1;
    }
    logd("create engine success");

    av_register_all();
    logd("test ffmpeg ok");
    return 0;
}

int talkback_stop(JNIEnv *env, jobject jobj) {
    return 0;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK)
        return -1;

    jclass clazz;
    const char * const className = "com/example/konga/Talkback";


    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        loge("cannot get class:%s\n", className);
        return -1;
    }

    JNINativeMethod jniMethods[] = {
            {"start", "()I", (void *) talkback_start},
            {"stop",  "()I", (void *) talkback_stop},
    };
    result = (*env)->RegisterNatives( env, clazz, jniMethods,
                                     sizeof(jniMethods) / sizeof(jniMethods[0]) );
    if (JNI_OK != result) {
        loge("register native method failed!\n");
        return -1;
    }

    return JNI_VERSION_1_6;
}
