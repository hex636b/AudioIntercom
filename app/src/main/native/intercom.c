//
// Created by konga on 2018/12/7.
//
#include <stdio.h>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>
#include <jni.h>
#include "libavformat/avformat.h"

#include "intercom.h"

#define TAG "intercom"
#define MLOGD(fmt, arg...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s:%u "fmt, __FUNCTION__, __LINE__, ##arg)
#define MLOGI(fmt, arg...)  __android_log_print(ANDROID_LOG_INFO,  TAG, "%s:%u "fmt, __FUNCTION__, __LINE__,  ##arg)
#define MLOGE(fmt, arg...)  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u "fmt, __FUNCTION__, __LINE__,  ##arg)


SLObjectItf engine;
SLEngineItf engineItf;

SLObjectItf recorder;
SLRecordItf recordItf;

SLDataLocator_AndroidSimpleBufferQueue  bufferQueue ;
SLAndroidSimpleBufferQueueItf bufferQueueItf;

char buffer[256] ;

static void BufferQueueCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    SLresult result;
    MLOGD("enqueue");
    (*bq)->Clear(bq);
    result = (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, sizeof(buffer));
}

int intercom_start(JNIEnv *env, jclass clazz)
{
    SLresult  result;
    SLEngineOption engineOption[] = {
            {(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE},
    };
    result = slCreateEngine(&engine, 1, engineOption, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("create engine fail %#X", result);
        return -1;
    }
    MLOGI("slCreateEngine");

    result = (*engine)->Realize(engine, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("realize engine fail %#X", result);
        return -1;
    }
    MLOGI("realize engine");


    SLDataLocator_IODevice ioDevice;
    SLDataSource audioSource;
    SLDataSink audioSink;

    result = (*engine)->GetInterface(engine, SL_IID_ENGINE, (void*)&engineItf);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("get SL_IID_ENGINE interface failed %#X", result);
        return -1;
    }

    /* Setup the data source structure */
    ioDevice.locatorType = SL_DATALOCATOR_IODEVICE;
    ioDevice.deviceType = SL_IODEVICE_AUDIOINPUT;
    ioDevice.deviceID = SL_DEFAULTDEVICEID_AUDIOINPUT;
    ioDevice.device = NULL;
    audioSource.pLocator = (void *)&ioDevice;
    audioSource.pFormat = NULL;


    bufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    bufferQueue.numBuffers = 2;
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,
            1,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_CENTER,
            SL_BYTEORDER_LITTLEENDIAN
    };
    audioSink.pLocator = &bufferQueue;
    audioSink.pFormat = &formatPcm;


    const SLInterfaceID interfaceID[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean interfaceReqired[1] = {SL_BOOLEAN_TRUE};
    /* Create audio recorder */
    result = (*engineItf)->CreateAudioRecorder(engineItf, &recorder, &audioSource, &audioSink, 1, interfaceID, interfaceReqired);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("CreateAudioRecorder failed %#X", result);
        return -1;
    }
    MLOGD("Create AudioRecorder");

    /* Realizing the recorder in synchronous mode. */
    result = (*recorder)->Realize(recorder, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("realize recorder faile %#X", result);
        return -1;
    }
    MLOGD("Realize AudioRecorder");
    /* Get the RECORD interface - it is an implicit interface */
    result = (*recorder)->GetInterface(recorder, SL_IID_RECORD, (void*)&recordItf);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("get SL_IID_RECORD interface failed %#X", result);
        return -1;
    }

    result = (*recorder)->GetInterface(recorder, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &bufferQueueItf);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("get SL_IID_ANDROIDSIMPLEBUFFERQUEUE interface failed %#X", result);
        return -1;
    }

    result = (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BufferQueueCallback, NULL);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("Register record event Callback failed %#X", result);
        return -1;
    }
    MLOGD("register BufferQueueCallback");


    /* Record the audio */
    result = (*recordItf)->SetRecordState(recordItf,SL_RECORDSTATE_RECORDING);
    MLOGD("set RECORDING");

    MLOGD("First enqueue");
    result = (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, sizeof(buffer));
}


int intercom_stop(JNIEnv *env, jclass clazz) {

    if (NULL != recorder) {
        MLOGD("destroy recorder");
        (*recorder)->Destroy(recorder);
        recorder = NULL;
    }
    if (NULL != engine) {
        MLOGD("destroy engine");
        (*engine)->Destroy(engine);
        engine = NULL;
    }
    return 0;
}


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jclass intercomClass = (*env)->FindClass(env, "com/github/hex636b/AudioRecord");
    if (NULL == intercomClass) {
        MLOGE("cannot find class AudioRecord");
        return -1;
    }

    JNINativeMethod jniMethods[] = {
            {"start",  "()I", (void *) intercom_start},
            {"stop",  "()I", (void *) intercom_stop},
    };
    result = (*env)->RegisterNatives(env, intercomClass, jniMethods,
                                     sizeof(jniMethods) / sizeof(jniMethods[0]) );
    if (JNI_OK != result) {
        MLOGE("register native method failed %#X", result);
        return -1;
    }

    return JNI_VERSION_1_6;
}
