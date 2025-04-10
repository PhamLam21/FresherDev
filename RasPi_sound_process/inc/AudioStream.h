/*******************************************************************************
*
*				Audio Framework
*				---------------
*
********************************************************************************
*	  AudioStream.h
********************************************************************************
*
*	  Description:	ALSA wrapper used for AWE reference examples
*                    AudioStream is a lightweight ALSA wrapper. It is not intended to replace a
*                    production-ready framework, but rather to implement a simple way to encapsulate
*                    the successful execution paths in an audio device.
*                    
*	  Copyright:	(c) 2007-2023 DSP Concepts, Inc. All rights reserved.
*					3235 Kifer Road
*					Santa Clara, CA 95054
*
*******************************************************************************/

#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <alsa/asoundlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdbool.h>

typedef enum AudioStream_SampleFormat { 
    AudioStream_SampleFormat_S16_LE,
    AudioStream_SampleFormat_S32_LE
} AudioStream_SampleFormat;

// This is what the callback will return
typedef enum AudioStream_CallbackResult { 
    AudioStream_CallbackResult_Continue,
    AudioStream_CallbackResult_Stop,
    AudioStream_CallbackResult_Abort
} AudioStream_CallbackResult;

// This is the flag that will be passed into the callback
typedef enum AudioStream_CallbackFlag {
    AudioStream_CallbackFlag_Success,
    AudioStream_CallbackFlag_InputOverrun,
    AudioStream_CallbackFlag_InputOtherError,
    AudioStream_CallbackFlag_OutputUnderrun,
    AudioStream_CallbackFlag_OutputOtherError
} AudioStream_CallbackFlag;

typedef AudioStream_CallbackResult (*AudioStream_CallbackFunction) (
    const void *audioInputBuffer, void *audioOutputBuffer,
    unsigned long framesPerBuffer, AudioStream_CallbackFlag statusFlag,
    void *userData);

typedef struct AudioStream {
    char* inputDeviceName;
    char* outputDeviceName;
    unsigned int numInputChannels;
    unsigned int numOutputChannels;
    unsigned int sampleRate;
    AudioStream_SampleFormat sampleFormat;
    snd_pcm_format_t sampleFormatALSA;
    unsigned int sampleSize;
    unsigned int blockSize;

    snd_pcm_t *captureHandle;
    snd_pcm_t *playbackHandle;

    pthread_t threadHandle;

    bool seeedDeviceIdentified;

    int32_t *hardwareInputBuffer;
    int32_t *seeedHardwareOutputBuffer;
    int32_t *callbackInputBuffer;
    int32_t *callbackOutputBuffer;

    AudioStream_CallbackFunction callback;

    bool stopRequested;

    void *callbackData;
} AudioStream;

AudioStream* AudioStream_create();

void AudioStream_open(AudioStream* as, char* inputDeviceName, unsigned int numInputChannels,
    char* outputDeviceName, unsigned int numOutputChannels,
    AudioStream_SampleFormat format, unsigned int sampleRate,
    unsigned int blockSize, AudioStream_CallbackFunction callbackFunction,
    void *userData);

void AudioStream_close(AudioStream* as);

void AudioStream_start(AudioStream* as);

void AudioStream_stop(AudioStream* as);


#endif  // AUDIO_STREAM_H