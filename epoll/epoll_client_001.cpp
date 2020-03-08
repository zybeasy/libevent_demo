#include <iostream>

#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

using namespace std;

#define MAX_LINE (1024)
#define SERVER_PORT ((7778)

void set_noblocking(int fd) {
    int opts = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, opts | O_NONBLOCK);
}

void func(const char* server_ip) {
    int sockfd;
    char recvline[MAX_LINE + 1] = {0};

    struct sockaddr_in server_addr;

    if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(0);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error for ");
        fprintf(stderr, "inet_pton error for %s", server_ip);
        exit(0);
    }

    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        exit(0);
    }

    set_noblocking(sockfd);

    char input[100];
    int n = 0;
    int count = 0;

    while(fgets(input, 100, stdin) != NULL) {
        cout << "[send] " << input << endl;
        n = send(sockfd, input, strlen(input), 0);
        if(n < 0) {
            perror("send");
        }

        while(1) {
            n = read(sockfd, recvline+count, MAX_LINE);
            if(n == MAX_LINE) {
                count += n;
                continue;
            }
            else if (n < 0) {
                perror("recv");
                break;
            }
            else {
                count += n;
                recvline[count] = '\0';
                cout << "[recv]: " << recvline << endl;
                break;
            }
        }
    }
}