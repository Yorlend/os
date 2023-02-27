#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>

char msg[] = "aaa";

#define SOCK_PATH "./server.sock"

int main()
{
    int fd;
    pid_t pid;
    char buf[1024];
    struct sockaddr server_value;
    
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    server_value.sa_family = AF_UNIX;
    strcpy(server_value.sa_data, SOCK_PATH);

    pid = getpid();
    sprintf(buf, "%d", pid);

    if (sendto(fd, buf, strlen(buf), 0, &server_value, sizeof(server_value)) == -1)
    {
        perror("sendto");
        exit(1);
    }

    close(fd);

    return 0;
}
