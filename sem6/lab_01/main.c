#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define STR2 "Message received"
#define BUF_SIZE 1024
#define CHILD_COUNT 5

int main(int argc, char **argv)
{
    int sockets[2];
    char buf[BUF_SIZE];
    int pid;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
    {
        perror("socketpair() failed");
        return 1;
    }

    for (int i = 0; i < CHILD_COUNT; i++)
    {
        if ((pid = fork()) == -1)
        {
            perror("can't fork");
            return 1;
        }

        if (pid == 0)
        {
            int len;
            close(sockets[1]);
            len = sprintf(buf, "Message from child %d", i);
            write(sockets[0], buf, len + 1);
            read(sockets[0], buf, sizeof(buf));
            printf("%s\n", buf);
            close(sockets[0]);

            exit(0);
        }
    }

    close(sockets[0]);
    read(sockets[1], buf, sizeof(buf));
    printf("%s\n", buf);
    write(sockets[1], STR2, sizeof(STR2));
    close(sockets[1]);
}