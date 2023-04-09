#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>

#define MAX_CONN        20
#define MAX_EVENTS      10
#define MAX_LEN         128
#define LISTEN_PORT     8080

int main(void)
{
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;
    char buf_recv[MAX_LEN], buf_send[MAX_LEN];

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(LISTEN_PORT),
        .sin_addr = htonl(INADDR_LOOPBACK)
    };

    struct sockaddr client_addr;
    socklen_t client_addrlen;

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    if (bind(listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(listen_sock, MAX_CONN) == -1)
    {
        perror("listen");
        exit(1);
    }

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(1);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(1);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(1);
        }

        for (int n = 0; n < nfds; n++)
        {
            if (events[n].data.fd == listen_sock)
            {
                conn_sock = accept(listen_sock, &client_addr, &client_addrlen);

                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(1);
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                fcntl(conn_sock, F_SETFL, O_NONBLOCK);
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    exit(1);
                }
            }
            else
            {
                if (recv(events[n].data.fd, buf_recv, sizeof(buf_recv), 0) == -1)
                {
                    perror("recv");
                    exit(1);
                }
                printf("Server with pid %d received: %s\n", getpid(), buf_recv);

                sprintf(buf_send, "%d received %s\n", getpid(), buf_recv);

                if (send(events[n].data.fd, buf_send, sizeof(buf_send), 0) == -1)
                {
                    perror("send");
                    exit(1);
                }

                close(events[n].data.fd);
            }
        }
    }
}
