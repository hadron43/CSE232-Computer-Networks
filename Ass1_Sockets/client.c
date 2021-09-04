#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "processes.c"

int main(int argc, char *argv[]) {
    int sd, tmp, len, fd;
    struct sockaddr_in server;
    char s_buff[1024];
    char filename[64];

    if(argc < 4) {
        puts("Error! Usage: ./client.out <server_ip> <port_number> N\n");
        exit(-1);
    }

    // Create a socket
    sd = socket(PF_INET, SOCK_STREAM, 0);
    check_error(sd, "socket");

    // intialize filename
    sprintf(filename, "client_%d.dat", sd);

    // Initialize server
    server.sin_family = PF_INET;
    server.sin_port = htons(atoi(argv[2]));
    tmp = inet_pton(AF_INET, argv[1], (void *)&server.sin_addr.s_addr);
    check_error(tmp, "invalid ip");

    len = sizeof(server);
    connect(sd, (struct sockaddr *)&server, len);

    fd = open(filename, O_WRONLY | O_CREAT);
    if(fd < 0) {
        perror("fopen");
        exit(-1);
    }

    // send N from command line to server
    write(sd, argv[3], sizeof(argv[3]));

    // in reply, server will now send top N processes details
    // read until server stops sending
    while(read(sd, s_buff, sizeof(s_buff)) > 0) {
        if(check_end(s_buff) != 0)
            break;
        tmp = write(fd, s_buff, strlen(s_buff));
        check_error(tmp, "write");
    }
    close(fd);

    // now client will find it's highest cpu consuming process
    // read the processes and prepare a file
    sprintf(filename, "client_for_server_%d.out", sd);
    create_data_file(filename, 1);

    // send the file to server
    printf("Data created, sending data to server.\n");

    // read from file and write to socket
    fd = open(filename, O_RDONLY);
    strcpy(s_buff, "");
    while((tmp = read(fd, s_buff, sizeof(s_buff))) > 0) {
        s_buff[tmp] = '\0';
        printf("file: %s, %d, sending: %s\n", filename, tmp, s_buff);
        write(sd, s_buff, tmp + 1);
    }
    // send end message
    send_end(sd);

    // remove(filename);
    close(fd);
    close(sd);
}