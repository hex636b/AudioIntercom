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

#define MAX_NUMBER_INTERFACES 5
#define MAX_NUMBER_INPUT_DEVICES 3
#define POSITION_UPDATE_PERIOD 1000 /* 1 sec */

static jclass g_intercomClass;
static jfieldID g_nativeInstanceID;

void RecordEventCallback(SLRecordItf caller, void * pContext, SLuint32 recordEvent)
{
    MLOGD("recordEvent %u", recordEvent);
}

int intercom_start(JNIEnv *env, jobject jobj)
{
    SLObjectItf engine = (*env)->GetLongField(env, jobj, g_nativeInstanceID);
    if (NULL == engine) {
        MLOGE("get engine fail. instanceID: %lld", g_nativeInstanceID);
        return -1;
    }

    SLObjectItf recorder;
    SLRecordItf recordItf;
    SLEngineItf engineItf;

    int i;
    SLresult result;
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

    SLDataLocator_AndroidSimpleBufferQueue  bufferQueue ;
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
    /* Setup to receive position event callbacks */
    result = (*recordItf)->RegisterCallback(recordItf, RecordEventCallback, NULL);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("Register record event Callback failed %#X", result);
        return -1;
    }
    /* Set notifications to occur after every second - may be useful in updating a recording progress bar */
    result = (*recordItf)->SetPositionUpdatePeriod( recordItf, POSITION_UPDATE_PERIOD);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("get SL_IID_RECORD interface failed %#X", result);
        return -1;
    }
    result = (*recordItf)->SetCallbackEventsMask( recordItf, SL_RECORDEVENT_HEADATNEWPOS);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("SetCallbackEventsMask failed %#X", result);
        return -1;
    }
    /* Set the duration of the recording - 30 seconds (30,000 milliseconds) */
    result = (*recordItf)->SetDurationLimit(recordItf, 30000);
    if (SL_RESULT_SUCCESS != result) {
        MLOGE("SetDurationLimit failed %#X", result);
        return -1;
    }

    /* Record the audio */
    result = (*recordItf)->SetRecordState(recordItf,SL_RECORDSTATE_RECORDING);
    /* Destroy the recorder object */
    (*recorder)->Destroy(recorder);
}

int intercom_create(JNIEnv *env, jobject jobj) {
    SLresult result;
    SLObjectItf engine;

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

    (*env)->SetLongField(env, jobj, g_nativeInstanceID, engine);

    return 0;
}

int intercom_destroy(JNIEnv *env, jobject jobj) {

    SLObjectItf engine = (*env)->GetLongField(env, jobj, g_nativeInstanceID);
    if (NULL != engine) {
        MLOGD("destroy engine");
        (*engine)->Destroy(engine);
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

    g_intercomClass = (*env)->FindClass(env, "com/example/konga/AudioIntercom");
    if (g_intercomClass == NULL) {
        MLOGE("cannot find class AudioIntercom");
        return -1;
    }
    g_nativeInstanceID = (*env)->GetFieldID(env, g_intercomClass, "nativeInstance", "J");

    JNINativeMethod jniMethods[] = {
            {"create", "()I", (void *) intercom_create},
            {"start",  "()I", (void *) intercom_start},
            {"destroy",  "()I", (void *) intercom_destroy},
    };
    result = (*env)->RegisterNatives(env, g_intercomClass, jniMethods,
                                     sizeof(jniMethods) / sizeof(jniMethods[0]) );
    if (JNI_OK != result) {
        MLOGE("register native method failed %#X", result);
        return -1;
    }

    return JNI_VERSION_1_6;
}
