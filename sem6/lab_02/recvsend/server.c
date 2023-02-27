#include <sys/socket.h>
#include <signal.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKET_PATH "./server.sock"

int fd;

void sigint_handler(int sig);

int main(void)
{
    signal(SIGINT, sigint_handler);

    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
        .sun_path = SOCKET_PATH
    };

    char buf[256];
    struct sockaddr client_addr;
    socklen_t len;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        perror("socket");
    else if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        printf("cannot bind\n");
    else
    {
        printf("bind successful. Ready to listen...\n");

        if (listen(fd, 1) != 0) {
            perror("listen");
        }

        while (1)
        {
            int sock = accept(fd, &client_addr, &len);

            if (recv(sock, buf, 256, 0) == -1)
            {
                perror("recvfrom");
                exit(1);
            }

            printf("received request: \"%s\"\n", buf);
            int pid = atoi(buf + 17);

            snprintf(buf, sizeof(buf), "response for pid=%d", pid);

            if (send(sock, buf, sizeof(buf), 0) == -1)
            {
                printf("error when sending response for pid=%d\n", pid);
                perror("sendto");
            }
            else
                printf("sent response for pid=%d\n", pid);
        }
    }

    return -1;
}

void sigint_handler(int sig)
{
    close(fd);
    remove(SOCKET_PATH);
    printf("\nRemoved socket file.\n");
    exit(0);
}
