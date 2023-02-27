#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVER_SOCKET_PATH "./server.sock"
#define CLIENT_SOCKET_PATH "./client.sock"

int main(void)
{
    int fd;
    struct sockaddr_un client_addr = {
        .sun_family = AF_UNIX,
        .sun_path = SERVER_SOCKET_PATH
    };

    char buf[256];

    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
        .sun_path = CLIENT_SOCKET_PATH
    };

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(1);
    }
    else if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
    }
    else if (connect(fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1)
    {
        perror("connect");
    }
    else
    {
        printf("sending request...\n");
        snprintf(buf, sizeof(buf), "request from pid=%d", getpid());

        if (send(fd, buf, sizeof(buf), 0) == -1)
        {
            perror("sendto");
            exit(1);
        }

        printf("sent request from pid=%d\n", getpid());

        if (recv(fd, buf, sizeof(buf), 0) == -1)
            perror("recvfrom");
        else
            printf("received response: \"%s\"\n", buf);
    }

    close(fd);
    remove(CLIENT_SOCKET_PATH);
    return 0;
}
