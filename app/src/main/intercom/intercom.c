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
#define logd(fmt, arg...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s:%u "fmt, __FUNCTION__, __LINE__, ##arg)
#define logi(fmt, arg...)  __android_log_print(ANDROID_LOG_INFO,  TAG, "%s:%u "fmt, __FUNCTION__, __LINE__,  ##arg)
#define loge(fmt, arg...)  __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u "fmt, __FUNCTION__, __LINE__,  ##arg)

#define MAX_NUMBER_INTERFACES 5
#define MAX_NUMBER_INPUT_DEVICES 3
#define POSITION_UPDATE_PERIOD 1000 /* 1 sec */

static jclass g_intercomClass;
static jfieldID g_nativeInstanceID;

void RecordEventCallback(SLRecordItf caller, void * pContext, SLuint32 recordEvent)
{
    logd("recordEvent %u", recordEvent);
}

int intercom_start(JNIEnv *env, jobject jobj)
{
    SLObjectItf engineObjItf = (*env)->GetLongField(env, jobj, g_nativeInstanceID);
    if (NULL == engineObjItf) {
        loge("get engineObjItf fail. instanceID: %lld", g_nativeInstanceID);
        return -1;
    }

    SLObjectItf recorder;
    SLRecordItf recordItf;
    SLEngineItf EngineItf;
    SLAudioIODeviceCapabilitiesItf AudioIODeviceCapabilitiesItf;
    SLAudioInputDescriptor AudioInputDescriptor;
    SLresult result;
    SLDataSource audioSource;
    SLDataLocator_IODevice locator_mic;
    SLDeviceVolumeItf devicevolumeItf;
    SLDataSink audioSink;
    SLDataLocator_URI uri;
    SLDataFormat_MIME mime;
    int i;
    SLboolean required[MAX_NUMBER_INTERFACES];
    SLInterfaceID iidArray[MAX_NUMBER_INTERFACES];
    SLuint32 InputDeviceIDs[MAX_NUMBER_INPUT_DEVICES];
    SLint32 numInputs = 0;
    SLboolean mic_available = SL_BOOLEAN_FALSE;
    SLuint32 mic_deviceID = 0;

    /* Get the SL Engine Interface which is implicit */
    result = (*engineObjItf)->GetInterface(engineObjItf, SL_IID_ENGINE, (void*)&EngineItf);
    if (SL_RESULT_SUCCESS != result) {
        loge("get SL_IID_ENGINE interface failed");
        return -1;
    }

    /* Get the Audio IO DEVICE CAPABILITIES interface, which is also implicit */
    result = (*engineObjItf)->GetInterface(engineObjItf, SL_IID_AUDIOIODEVICECAPABILITIES, (void*)&AudioIODeviceCapabilitiesItf);
    if (SL_RESULT_SUCCESS != result) {
        loge("get SL_IID_AUDIOIODEVICECAPABILITIES interface failed");
        return -1;
    }
    numInputs = MAX_NUMBER_INPUT_DEVICES;
    result = (*AudioIODeviceCapabilitiesItf)->GetAvailableAudioInputs( AudioIODeviceCapabilitiesItf, &numInputs, InputDeviceIDs);
    if (SL_RESULT_SUCCESS != result) {
        loge("get GetAvailableAudioInputs failed");
        return -1;
    }

    /* Search for either earpiece microphone or headset microphone input device - with a preference for the latter */
    for (i=0;i<numInputs; i++)
    {
        result = (*AudioIODeviceCapabilitiesItf)->QueryAudioInputCapabilities(AudioIODeviceCapabilitiesItf, InputDeviceIDs[i], &AudioInputDescriptor);
        if (SL_RESULT_SUCCESS != result) {
            loge("get QueryAudioInputCapabilities failed");
            return -1;
        }
        if((AudioInputDescriptor.deviceConnection == SL_DEVCONNECTION_ATTACHED_WIRED)&&
           (AudioInputDescriptor.deviceScope == SL_DEVSCOPE_USER)&&
           (AudioInputDescriptor.deviceLocation == SL_DEVLOCATION_HEADSET))
        {
            mic_deviceID = InputDeviceIDs[i];
            mic_available = SL_BOOLEAN_TRUE;
            break;
        }
        else if((AudioInputDescriptor.deviceConnection == SL_DEVCONNECTION_INTEGRATED)&&
                (AudioInputDescriptor.deviceScope == SL_DEVSCOPE_USER)&&
                (AudioInputDescriptor.deviceLocation == SL_DEVLOCATION_HANDSET))
        {
            mic_deviceID = InputDeviceIDs[i];
            mic_available = SL_BOOLEAN_TRUE;
            break;
        }
    }
    /* If neither of the preferred input audio devices is available, no point in continuing */
    if (!mic_available) {
        loge(" ! mic_available");
        return -1;
    }
    /* Initialize arrays required[] and iidArray[] */
    for (i=0;i<MAX_NUMBER_INTERFACES;i++)
    {
        required[i] = SL_BOOLEAN_FALSE;
        iidArray[i] = SL_IID_NULL;
    }

    /* Get the optional DEVICE VOLUME interface from the engine */
    result = (*engineObjItf)->GetInterface(engineObjItf, SL_IID_DEVICEVOLUME, (void*)&devicevolumeItf);
    if (SL_RESULT_SUCCESS != result) {
        loge("get SL_IID_DEVICEVOLUME interface failed");
        return -1;
    }

/* Set recording volume of the microphone to -3 dB */
    result = (*devicevolumeItf)->SetVolume(devicevolumeItf, mic_deviceID, -300);
    if (SL_RESULT_SUCCESS != result) {
        loge("SetVolume failed");
        return -1;
    }
/* Setup the data source structure */
    locator_mic.locatorType = SL_DATALOCATOR_IODEVICE;
    locator_mic.deviceType = SL_IODEVICE_AUDIOINPUT;
    locator_mic.deviceID = mic_deviceID;
    locator_mic.device = NULL;
    audioSource.pLocator = (void *)&locator_mic;
    audioSource.pFormat = NULL;
    /* Setup the data sink structure */
    uri.locatorType = SL_DATALOCATOR_URI;
    uri.URI = (SLchar *) "file:///sdcard/recordsample.wav";
    mime.formatType = SL_DATAFORMAT_MIME;
    mime.mimeType = (SLchar *) "audio/x-wav";
    mime.containerType = SL_CONTAINERTYPE_WAV;
    audioSink.pLocator = (void *)&uri;
    audioSink.pFormat = (void *)&mime;
/* Create audio recorder */
    result = (*EngineItf)->CreateAudioRecorder(EngineItf, &recorder, &audioSource, &audioSink, 0, iidArray, required);
    if (SL_RESULT_SUCCESS != result) {
        loge("CreateAudioRecorder failed");
        return -1;
    }

/* Realizing the recorder in synchronous mode. */
    result = (*recorder)->Realize(recorder, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        loge("realize recorder failed");
        return -1;
    }
/* Get the RECORD interface - it is an implicit interface */
    result = (*recorder)->GetInterface(recorder, SL_IID_RECORD, (void*)&recordItf);
    if (SL_RESULT_SUCCESS != result) {
        loge("get SL_IID_RECORD interface failed");
        return -1;
    }
/* Setup to receive position event callbacks */
    result = (*recordItf)->RegisterCallback(recordItf, RecordEventCallback, NULL);
    if (SL_RESULT_SUCCESS != result) {
        loge("Register record event Callback failed");
        return -1;
    }
/* Set notifications to occur after every second - may be useful in updating a recording progress bar */
    result = (*recordItf)->SetPositionUpdatePeriod( recordItf, POSITION_UPDATE_PERIOD);
    if (SL_RESULT_SUCCESS != result) {
        loge("get SL_IID_RECORD interface failed");
        return -1;
    }
    result = (*recordItf)->SetCallbackEventsMask( recordItf, SL_RECORDEVENT_HEADATNEWPOS);
    if (SL_RESULT_SUCCESS != result) {
        loge("SetCallbackEventsMask failed");
        return -1;
    }
/* Set the duration of the recording - 30 seconds (30,000 milliseconds) */
    result = (*recordItf)->SetDurationLimit(recordItf, 30000);
    if (SL_RESULT_SUCCESS != result) {
        loge("SetDurationLimit failed");
        return -1;
    }

    /* Record the audio */
    result = (*recordItf)->SetRecordState(recordItf,SL_RECORDSTATE_RECORDING);
    /* Destroy the recorder object */
    (*recorder)->Destroy(recorder);
}

int intercom_create(JNIEnv *env, jobject jobj) {
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
    result = (*engineObjItf)->Realize(engineObjItf, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        loge("realize engine fail");
        return -1;
    }
    logd("create engine, got object interface");

    (*env)->SetLongField(env, jobj, g_nativeInstanceID, engineObjItf);

    return 0;
}

int intercom_destroy(JNIEnv *env, jobject jobj) {

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

    g_intercomClass = (*env)->FindClass(env, "com/example/konga/AudioIntercom");
    if (g_intercomClass == NULL) {
        loge("cannot find class AudioIntercom");
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
        loge("register native method failed!\n");
        return -1;
    }

    return JNI_VERSION_1_6;
}
