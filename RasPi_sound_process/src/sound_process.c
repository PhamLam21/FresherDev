#include"../inc/sound_process.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_reader = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_main = PTHREAD_COND_INITIALIZER;

const void* moduleDescriptorTable[] = {
    LISTOFCLASSOBJECTS
};
UINT32 moduleDescriptorTableSize = sizeof(moduleDescriptorTable) / sizeof(moduleDescriptorTable[0]);

int init_pcm(PCM_device_t *device) {
    printf("Initializing pcm device...\n");
    int err = snd_pcm_open(&device->dev, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(err < 0) {
        fprintf(stderr, "snd_pcm_open: can't open device\n", snd_strerror(err));
        return -1;
    }
    err = snd_pcm_set_params( device->dev, 
                              SND_PCM_FORMAT_S32_LE,
                              SND_PCM_ACCESS_RW_INTERLEAVED,
                              AWE_OUT_CHANNELS,
                              AWE_SAMPLE_RATE,
                              1,
                              100000 );
    if(err < 0) {
        fprintf(stderr, "snd_pcm_set_parameter: can't set parameter\n", snd_strerror(err));
        return -1;
    }
    return 0;
}

int init_aweCoreOS(const char* file) {
    printf("Initializing AWECoreOS...\n");
    AWEOSConfigParameters config;
    aweOS_getParamDefaults(&config);
    config.inChannels = AWE_IN_CHANNELS;
    config.outChannels = AWE_OUT_CHANNELS;
    config.sampleRate = AWE_SAMPLE_RATE;
    config.fundamentalBlockSize = AWE_BLOCK_SIZE;
    config.numThreads = 4;

    int ret = aweOS_init(&awe, &config, moduleDescriptorTable, moduleDescriptorTableSize);
    if(ret < 0) {
        fprintf(stderr, "aweOS_init: can't init aweOS %d\n", ret);
    }
    UINT32 pos;
    ret = aweOS_loadAWBFile(awe, file, &pos);
    if (ret != 0) {
        fprintf(stderr, "Failed to load AWB graph at pos %u: %s\n", pos, aweOS_errorToString(ret));
        return -1;
    }
    INT32 tuningRet = aweOS_tuningSocketOpen(&awe, AWE_PORT_NO, 1); 
    if (tuningRet < 0)
    {
        printf("Failing opening tuning interface with error %s \n", aweOS_errorToString(tuningRet));
    }
    else 
    {
        printf("Opened TCP tuning interface on port %d: Waiting for AWE Server Connection from PC... \n", AWE_PORT_NO);
    }

    return 0;
}

int init_TCPSocket(int *server_fd) {
    printf("Init TCPSocket.............\n");
    int opt = 1;
    struct  sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*server_fd < 0) {
        fprintf(stderr, "Failed to create tcp socket!\n", strerror(*server_fd));
        return -1;
    }

    /*Prevent error: "address already in use"*/
    if(setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        fprintf(stderr, "Failed setsocket()\n", strerror(-1));
        return -1;
    }

    /*Init server_addr*/
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT_NO);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(*server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Failed bind()\n", strerror(-1));
        return -1;
    }
    
    if(listen(*server_fd, 1) == -1) {
        fprintf(stderr, "Failed listen()\n", strerror(-1));
        return -1;
    }

    return 0;
}

void *read_thread(void* arg) {
    Read_file_t *cfg = (Read_file_t *)arg;
    // printf("Started read thread for channel offset: %d\n", cfg->channel_offset);
    FILE* file = fopen(cfg->file, "rb");
    if (!file) {
        perror("fopen");
    }
    //printf("thread %d", cfg->channel_offset);
    INT32 temp[AWE_BLOCK_SIZE * 2];  // stereo interleaved

    while (fread(temp, sizeof(INT32), AWE_BLOCK_SIZE * 2, file) == AWE_BLOCK_SIZE * 2) {
        pthread_mutex_lock(&mutex);
        // Wait if channels are full
        while (ready_channels[cfg->channel_offset] && ready_channels[cfg->channel_offset + 1]) {
            pthread_cond_wait(&cond_reader, &mutex);
        }
        // Deinterleave
        for (int i = 0; i < AWE_BLOCK_SIZE; ++i) {
            input_channels[cfg->channel_offset][i]     = temp[2 * i];     // Left
            input_channels[cfg->channel_offset + 1][i] = temp[2 * i + 1]; // Right
        }

        // Mark ready
        ready_channels[cfg->channel_offset] = 1;
        ready_channels[cfg->channel_offset + 1] = 1;
        pthread_cond_signal(&cond_main);
        pthread_mutex_unlock(&mutex);
    }

    fclose(file);
}

void *sound_processing(void *arg) {
    PCM_device_t *device = (PCM_device_t *) arg;
    while(1) {
        pthread_mutex_lock(&mutex);
        while(!(ready_channels[0] && ready_channels[1] && ready_channels[2] && ready_channels[3])) {
            pthread_cond_wait(&cond_main, &mutex);
        }

        //Import AWE
        for(int ch = 0; ch < AWE_IN_CHANNELS; ch++) {
            aweOS_audioImportSamples(awe, input_channels[ch], 1, ch, AWE_SAMPLE_TYPE);
        }

        memset(ready_channels, 0, sizeof(ready_channels));
        pthread_cond_broadcast(&cond_reader);
        pthread_mutex_unlock(&mutex);

        //Pump
        aweOS_audioPumpAll(awe);

        //Export for PCM device
        for(int ch = 0; ch < AWE_OUT_CHANNELS; ch++) {
            aweOS_audioExportSamples(awe, output_channels + ch, AWE_OUT_CHANNELS, ch, AWE_SAMPLE_TYPE);
        }

        //Write to device
        int frames = snd_pcm_writei(device->dev, output_channels, AWE_BLOCK_SIZE);
        if(frames < 0) {
            frames = snd_pcm_recover(device->dev, frames, 0);
            if(frames < 0) {
                fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror(frames));
                break;
            }
        }
    }
}

void socket_chat(int client_fd) {
    int numb_read;
    char recvbuff[TCP_BUFF_SIZE];

    while (1)
    {
        memset(recvbuff, 0, TCP_BUFF_SIZE);
        
        numb_read = recv(client_fd, recvbuff, TCP_BUFF_SIZE, 0);

        if (numb_read <= 0) {
            if (numb_read == 0)
                printf("Client disconnected.\n");
            else
                fprintf(stderr, "Failed recv()\n", strerror(-1));
            break;
        }

        recvbuff[numb_read] = '\0';
        recvbuff[strcspn(recvbuff, "\r\n")] = 0; 
        printf("\nMessage from client: %s\n", recvbuff);

        if (strncmp("exit", recvbuff, 4) == 0) {
            system("clear");
            break;
        } else if (strncmp("set ", recvbuff, 4) == 0) {
            int objectID;
            float newVal;
            if (sscanf(recvbuff + 4, "%d %f", &objectID, &newVal) == 2) {
                if(objectID == 30001) {
                    if(newVal < -60 || newVal > 24) {
                        send(client_fd, "Invalid value\n", 14, 0);
                    } else {
                        aweOS_ctrlSetValue(awe, AWE_ScalerN2_masterGain_HANDLE, &newVal, 0, AWE_ScalerN2_masterGain_SIZE);
                        send(client_fd, "OK\n", 3, 0);
                    }
                } else if (objectID == 30002) {
                    int val = (int) newVal;
                    if(val != 0 && val != 1) {
                        send(client_fd, "Invalid value\n", 14, 0);
                    } else {
                        aweOS_ctrlSetValue(awe, AWE_Mute1_isMuted_HANDLE, &val, 0, AWE_Mute1_isMuted_SIZE);
                        send(client_fd, "OK\n", 3, 0);
                    }
                } else {
                    send(client_fd, "Invalid object\n", 14, 0);
                }
            } else {
                send(client_fd, "Invalid format\n", 15, 0);
            }
        } else {
            send(client_fd, "Unknown command\n", 16, 0);
        }
    }
    close(client_fd);
}
