#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "processes.c"

void *handle_connection(void *arg) {
    int connfd = *((int *)arg), rv, fd;
    char r_buff[1024];

    // Receive N from client
    // N tells the number of processes for which data is to be returned
    rv = read(connfd, r_buff, sizeof(r_buff));
    check_error(rv, "read");

    // parse the string to an integer
    int N = atoi(r_buff);
    printf("N: %d\n", N);

    // read the processes and prepare a file
    char filepath[256];
    sprintf(filepath, "server_%d.out", connfd);
    create_data_file(filepath, N);

    printf("Data created for sd %d, sending data to client.\n", connfd);

    // read from file and write to socket
    fd = open(filepath, O_RDONLY);
    while(read(fd, r_buff, sizeof(r_buff)) > 0) {
        write(connfd, r_buff, sizeof(r_buff));
    }
    // send end message
    send_end(connfd);
    close(fd);

    remove(filepath);

    // in reply, client will now send highest cpu consuming process
    // read until server stops sending
    while(read(connfd, r_buff, sizeof(r_buff)) > 0) {
        if(check_end(r_buff) != 0)
            break;
        printf("reply from client sd %d: %s\n", connfd, r_buff);
    }

    // busy wait
    for(int i = 0; i < 999999999; ++i);
    printf("closing connection to client with fd %d.\n", connfd);
    close(connfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int sd, connfd, tmp, len;
    struct sockaddr_in myaddr, client_addr;

    // Create a socket
    sd = socket(PF_INET, SOCK_STREAM, 0);
    check_error(sd, "socket");

    // Initialize myaddr
    myaddr.sin_family = PF_INET;
    myaddr.sin_port = htons(80);
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to an address
    tmp = bind(sd, (struct sockaddr *)&myaddr, sizeof(myaddr));
    check_error(tmp, "bind");

    tmp = listen(sd, 50);
    check_error(tmp, "listen");

    printf("Listening for requests at port 80...\n\n");

    while(connfd = accept(sd, (struct sockaddr *)&client_addr, &len)) {
        check_error(connfd, "accept");
        printf("connected to client %d at port %d. fd: %d\n",
            client_addr.sin_addr.s_addr, client_addr.sin_port, connfd);

        pthread_t *ptid = malloc(sizeof(pthread_t));
        int *sfd_for_thread = malloc(sizeof(int));
        (*sfd_for_thread) = connfd;

        pthread_create(ptid, NULL, &handle_connection, (void *)sfd_for_thread);
    }
}
