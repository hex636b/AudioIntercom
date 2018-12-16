//
// Created by konga on 2018/12/7.
//
#include <stdio.h>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <android/log.h>
#include <jni.h>
#include "libavformat/avformat.h"

#include "intercom.h"

#define TAG "intercom"
#define logd(fmt, arg...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##arg)
#define logi(fmt, arg...)  __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##arg)
#define loge(fmt, arg...)  __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##arg)

static jclass g_talkbackClass;
static jfieldID g_nativeInstanceID;

int intercom_start(JNIEnv *env, jobject jobj) {
    SLresult result;
    SLObjectItf engineObjItf;

    SLEngineOption engineOption[] = {
            {(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE},
    };
    result = slCreateEngine(&engineObjItf, 1, engineOption, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        loge("create engine fail");
        return -1;
    }
    logd("create engine, got object interface");

    (*env)->SetLongField(env, jobj, g_nativeInstanceID, engineObjItf);

    return 0;
}

int intercom_stop(JNIEnv *env, jobject jobj) {

    SLObjectItf engineObjItf = (*env)->GetLongField(env, jobj, g_nativeInstanceID);
    if (NULL != engineObjItf) {
        logd("destroy engine object intface");
        (*engineObjItf)->Destroy(engineObjItf);
        (*env)->SetLongField(env, jobj, g_nativeInstanceID, NULL);
    }
    return 0;
}


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    g_talkbackClass = (*env)->FindClass(env, "com/example/konga/AudioIntercom");
    if (g_talkbackClass == NULL) {
        loge("cannot find class AudioIntercom");
        return -1;
    }
    g_nativeInstanceID = (*env)->GetFieldID(env, g_talkbackClass, "nativeInstance", "J");

    JNINativeMethod jniMethods[] = {
            {"start", "()I", (void *) intercom_start},
            {"stop",  "()I", (void *) intercom_stop},
    };
    result = (*env)->RegisterNatives(env, g_talkbackClass, jniMethods,
                                     sizeof(jniMethods) / sizeof(jniMethods[0]) );
    if (JNI_OK != result) {
        loge("register native method failed!\n");
        return -1;
    }

    return JNI_VERSION_1_6;
}
