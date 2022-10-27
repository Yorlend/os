#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int flag = 0;

void status_handler(int);

void signal_handler(int);

int main()
{
    signal(SIGINT, signal_handler);

    printf("Press Ctrl+C to write to pipe.\n");
    sleep(4);

    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("Can't create pipe");
        return EXIT_FAILURE;
    }

    pid_t childpid1;
    if ((childpid1 = fork()) == -1)
    {
        perror("Can't fork");
        return EXIT_FAILURE;
    }
    if (childpid1 == 0)
    {
        close(fd[0]);
        if (flag)
        {
            char message[7 + sizeof(int)];
            *((int*)message) = 7;
            memcpy(message + sizeof(int), "qwerty", 7);
            write(fd[1], message, sizeof(message));
        }

        printf("\nChild 1:\n\tPID:\t%d\n\tPPID:\t%d\n\tPGRP:\t%d\n",
			getpid(), getppid(), getpgrp());
		return EXIT_SUCCESS;
    }

    pid_t childpid2;
    if ((childpid2 = fork()) == -1)
    {
        perror("Can't fork");
        return EXIT_FAILURE;
    }
    if (childpid2 == 0)
    {
        close(fd[0]);
        if (flag)
        {
            char message[9 + sizeof(int)];
            *((int*)message) = 9;
            memcpy(message + sizeof(int), "zaqwedsc", 9);
            write(fd[1], message, sizeof(message));
        }

        printf("\nChild 2:\n\tPID:\t%d\n\tPPID:\t%d\n\tPGRP:\t%d\n",
            getpid(), getppid(), getpgrp());
        return EXIT_SUCCESS;
    }

    int bufsize;
    char buf[32];
	int status;
	pid_t child_pid;

    close(fd[1]);
    

	child_pid = wait(&status);
	printf("\nstatus: %d\tchild_pid: %d\n", status, child_pid);
	status_handler(status);

	child_pid = wait(&status);
	printf("\nstatus: %d\tchild_pid: %d\n", status, child_pid);
	status_handler(status);

	printf("Parent:\n\tPID:\t%d\n\tPGRP:\t%d\n\tChild:\t%d\n",
			getpid(), getpgrp(), childpid1);

    status = read(fd[0], &bufsize, sizeof(int));

    if (status > 0)
    {
        read(fd[0], buf, bufsize);
        printf("Read message: %s\n", buf);
    }

    status = read(fd[0], &bufsize, sizeof(int));

    if (status > 0)
    {
        read(fd[0], buf, bufsize);
        printf("Read message: %s\n", buf);
    }
}

void status_handler(int status)
{
	if (WIFEXITED(status))
	{
		printf("Child process exited successfuly.\n");
	}
	else if (WEXITSTATUS(status))
	{
		printf("Child process exited with code: %d.\n", WEXITSTATUS(status));
	}
	else if (WIFSIGNALED(status))
	{
		printf("Child process ended abnormally (raised a signal that caused it to exit).\n");
		printf("Signal: %d.\n", WTERMSIG(status));
	}
	else if (WIFSTOPPED(status))
	{
		printf("Child process is currently stopped.\n");
		printf("Signal: %d.\n", WSTOPSIG(status));
	}
}

void signal_handler(int sig_num) {
    flag = 1;
    printf("\nsignal_handler is set for signal: %d\n", sig_num);
}
