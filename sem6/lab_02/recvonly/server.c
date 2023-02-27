#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define SOCK_PATH "./server.sock"

int fd;

void sigint_handler(int sig);

int main()
{
    char buf[1024];
    struct sockaddr server_name;

    signal(SIGINT, sigint_handler);

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(1);
    }

    server_name.sa_family = AF_UNIX;
    strcpy(server_name.sa_data, SOCK_PATH);

    if (bind(fd, &server_name, sizeof(server_name)) == -1)
    {
        perror("bind");
        exit(1);
    }

    while (1)
    {
        int bytes_read = read(fd, buf, 1024);
        if (bytes_read == -1)
        {
            perror("read");
            exit(1);
        }
        else
        {
            buf[bytes_read] = '\0';
            printf("Received: %s\n", buf);
        }
    }

    close(fd);
    printf("Removing socket file\n");
    remove(SOCK_PATH);
    return 0;
}

void sigint_handler(int sig)
{
    close(fd);
    remove(SOCK_PATH);
    printf("\nRemoved socket file.\n");
    exit(0);
}

