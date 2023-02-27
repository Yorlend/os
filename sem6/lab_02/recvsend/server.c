#include <sys/socket.h>
#include <signal.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKET_PATH "./server.sock"

int sock;

void sigint_handler(int sig);

void listen_loop(int sock)
{
    char buf[256];
    struct sockaddr client_addr;
    socklen_t len;

    while (1)
    {

        if (recvfrom(sock, buf, 256, 0, &client_addr, &len) == -1)
        {
            printf("error when receiving request\n");
            return;
        }

        printf("received request: \"%s\"\n", buf);
        int pid = atoi(buf + 17);

        snprintf(buf, sizeof(buf), "response for pid=%d", pid);

        if (sendto(sock, buf, sizeof(buf), 0, &client_addr, len) == -1)
            printf("error when sending response for pid=%d\n", pid);
        else
            printf("sent response for pid=%d\n", pid);
    }
}

int main(void)
{
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
        .sun_path = SOCKET_PATH
    };

    signal(SIGINT, sigint_handler);

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1)
        printf("failed to create socket\n");
    else if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        printf("cannot bind\n");
    else
    {
        printf("bind successful. Ready to listen...\n");

        listen(sock, 1);
        listen_loop(sock);
    }

    return -1;
}

void sigint_handler(int sig)
{
    close(sock);
    remove(SOCKET_PATH);
    printf("\nRemoved socket file.\n");
    exit(0);
}
