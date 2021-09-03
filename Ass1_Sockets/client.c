#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 1024

void check_error(int return_val, char *str) {
    if(return_val < 0) {
        perror(str);
        exit(return_val);
    }
}

int main(int argc, char *argv[]) {
    int sd, tmp, len, fd;
    struct sockaddr_in server;
    char s_buff[1024];
    char filename[64];

    if(argc < 2) {
        puts("Missing command line argument, N.");
        exit(-1);
    }

    // Create a socket
    sd = socket(PF_INET, SOCK_STREAM, 0);
    check_error(sd, "socket");

    // intialize filename
    sprintf(filename, "client_%d.dat", sd);

    // Initialize server
    server.sin_family = PF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    len = sizeof(server);
    connect(sd, (struct sockaddr *)&server, len);

    fd = open(filename, O_WRONLY | O_CREAT);
    if(fd < 0) {
        perror("fopen");
        exit(-1);
    }

    // send N from command line to server
    write(sd, argv[1], sizeof(argv[1]));

    // in reply, server will now send top N processes details
    // read until server stops sending
    while(read(sd, s_buff, sizeof(s_buff)) > 0) {
        tmp = write(fd, s_buff, strlen(s_buff));
        check_error(tmp, "write");
    }
    close(fd);
    close(sd);
}