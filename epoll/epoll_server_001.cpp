#include <iostream>

#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>
#include <sys/epoll.h>

using namespace std;

#define SERVER_PORT (7778)
#define EPOLL_MAX_NUM (2048)
#define BUFFER_MAX_LEN (4096)

char buffer[BUFFER_MAX_LEN];

void str_toupper(char *str) {
    for(int i; i<strlen(str); ++i)
        str[i] = toupper(str[i]);
}

void f() {
    int listen_fd = 0;
    int client_fd = 0;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len;

    int epfd = 0;
    struct epoll_event event, *my_events;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    listen(listen_fd, 10);

    // epoll create
    epfd = epoll_create(EPOLL_MAX_NUM);
    if(epfd < 0) {
        perror("epoll create");
        goto END;
    }

    // add listen_fd to epoll
    event.events = EPOLLIN;
    event.data.fd = listen_fd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
        perror("epoll ctl add listen_fd");
        goto END;
    }

    my_events = malloc(sizeof(struct epoll_event) * EPOLL_MAX_UNM);
    while(1) {
        int active_fds_cnt = epoll_wait(epfd, my_events, EPOLL_MAX_NUM, -1);
        for(int i=0; i<active_fds_cnt; ++i) {
            if(my_events[i].data.fd == listen_fd) {
                client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
                if(client_fd < 0) {
                    perror("accpet");
                    continue;
                }

                char ip[20];
                cout << "New connection ["
                << inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)) << ":"
                << ntohs(client_addr.sin_port) << "]" << endl;

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event);
            }
            else if(my_events[i].events & EPOLLIN) {
                cout << "EPOLLIN" << endl;
                client_fd = my_events[i].data.fd;
                buffer[0] = '\0';
                int n = read(client_fd, buffer, 5);
                if(n < 0) {
                    perror("read");
                    continue;
                }
                else if(n == 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &event);
                    close(client_fd);
                }
                else {
                    cout << "[read]: " << buffer << endl;
                    buffer[n] = '\0';
#if 1
                    str_toupper(buffer);
                    write(client_fd, buffer, strlen(buffer));
                    cout << "[write]: " << buffer << endl;
                    memset(buffer, 0, BUFFER_MAX_LEN);
#endif
                }
            }
            else if (my_events[i].events & EPOLLOUT) {
                cout << "EPOLLOUT" << endl;
            }
        }
    }

    END:
    close(epfd);
    close(listen_fd);

}