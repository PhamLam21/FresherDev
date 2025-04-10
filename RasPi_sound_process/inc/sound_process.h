#ifndef __SOUND_PROCESS_H__
#define __SOUND_PROCESS_H__

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/socket.h>    
#include <netinet/in.h>     
#include <arpa/inet.h>
#include"External/alsa/asoundlib.h"
#include"AWECoreOS.h"
#include"ModuleList.h"
#include"Kanavi_passthrouh_test_ControlInterface.h"

/*AWE process*/
#define AWE_IN_CHANNELS 4
#define AWE_OUT_CHANNELS 2
#define AWE_BLOCK_SIZE 768
#define AWE_SAMPLE_RATE 48000
#define AWE_SAMPLE_TYPE Sample32bit
#define AWE_PORT_NO 15002

/*TCP Socket*/
#define TCP_PORT_NO 24
#define TCP_BUFF_SIZE 64

/*Mutex, cond*/
extern pthread_mutex_t mutex;
extern pthread_cond_t cond_reader;
extern pthread_cond_t cond_main;

/*AWE init*/
AWEOSInstance *awe;
extern const void* moduleDescriptorTable[];
extern UINT32 moduleDescriptorTableSize;

//Buffer
INT32 input_channels[AWE_IN_CHANNELS][AWE_BLOCK_SIZE];
INT32 output_channels[AWE_BLOCK_SIZE * AWE_OUT_CHANNELS];
int ready_channels[AWE_IN_CHANNELS];

/*Device, file defination*/
typedef struct {
    snd_pcm_t *dev;
} PCM_device_t;

typedef struct {
    const char *file;
    int channel_offset; //0 or 2
} Read_file_t;

/*Function*/
int init_pcm(PCM_device_t *device);
int init_aweCoreOS(const char* file);
int init_TCPSocket(int *server_fd);
void *read_thread(void *arg);
void *sound_processing(void *arg);
void socket_chat(int client_fd);

#endif /*__SOUND_PROCESS_H__*/



