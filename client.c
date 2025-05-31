#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "cjson/cJSON.h"

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sockfd, status, opt;
    char buffer[1024];

    char ip[100], port[100], username[100];

   while ((opt = getopt(argc, argv, "ip:u:")) != -1) {
        switch(opt) {
            case 'ip':
                *ip = optarg;
                break;
            case 'port':
                *port = optarg;
                break;
            case 'username':
                *username = optarg;
                break;
        }
    }

    // Set up socket parameters
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the IP address of the remote server using getaddrinfo
    status = getaddrinfo(ip, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // Create a socket object and connect to the remote server
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket error");
        exit(1);
    }

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        perror("connect error");
        exit(1);
    }

    // Send a message to the server
    const char *message = "Hi There\n";
    send(sockfd, message, strlen(message), 0);

    // Receive the response from the server
    int bytes_received = recv(sockfd, buffer, sizeof buffer - 1, 0);
    buffer[bytes_received] = '\0';

    // Print the response from the server
    printf("%s", buffer);

    // Close the socket connection
    close(sockfd);

    return 0;
}

