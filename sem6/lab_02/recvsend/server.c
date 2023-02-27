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

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1)
        perror("socket");
    else if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        printf("cannot bind\n");
    else
    {
        printf("bind successful. Ready to listen...\n");

        listen(fd, 1);
        while (1)
        {
            if (recvfrom(fd, buf, 256, 0, &client_addr, &len) == -1)
            {
                perror("recvfrom");
                exit(1);
            }

            printf("received request: \"%s\"\n", buf);
            int pid = atoi(buf + 17);

            snprintf(buf, sizeof(buf), "response for pid=%d", pid);

            if (sendto(fd, buf, sizeof(buf), 0, &client_addr, len) == -1)
                printf("error when sending response for pid=%d\n", pid);
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
