#include"../inc/sound_process.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <input1.pcm> <input2.pcm> <graph.awb>\n", argv[0]);
        return 1;
    }
    init_aweCoreOS(argv[3]);

    PCM_device_t pcm_dev;
    if (init_pcm(&pcm_dev) != 0) {
        return 1;
    }

    Read_file_t read1 = {argv[1], 0};
    Read_file_t read2 = {argv[2], 2};

    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, read_thread, &read1);
    pthread_create(&thread2, NULL, read_thread, &read2);
    pthread_create(&thread3, NULL, sound_processing, &pcm_dev);

    int server_fd;
    int client_fd;
    int len;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));

    init_TCPSocket(&server_fd);
    len = sizeof(client_addr);

    while (1) {
        printf("Server is listening at port: %d \n..............\n", TCP_PORT_NO);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *)&len);
        if(client_fd == -1) 
            fprintf(stderr, "Failed accept()\n", strerror(-1));
        
        system("clear");

        printf("Server: got connection\n");
        socket_chat(client_fd);
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    aweOS_destroy(&awe);
    snd_pcm_close(pcm_dev.dev);
    close(server_fd);
    return 0;
}