#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void status_handler(int status);

int main()
{
	pid_t childpid1;
	if ((childpid1 = fork()) == -1)
	{
		perror("Can't fork\n");
		return EXIT_FAILURE;
	}
	if (childpid1 == 0)
	{
		printf("Child 1:\n\tPID:\t%d\n\tPPID:\t%d\n\tPGRP:\t%d\n",
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
		printf("Child 2:\n\tPID:\t%d\n\tPPID:\t%d\n\tPGRP:\t%d\n",
			getpid(), getppid(), getpgrp());
		return EXIT_SUCCESS;
	}

	int status;
	pid_t child_pid;

	child_pid = wait(&status);
	printf("\nstatus: %d\tchild_pid: %d\n", status, child_pid);
	status_handler(status);

	child_pid = wait(&status);
	printf("\nstatus: %d\tchild_pid: %d\n", status, child_pid);
	status_handler(status);

	printf("Parent:\n\tPID:\t%d\n\tPGRP:\t%d\n\tChild:\t%d\n",
			getpid(), getpgrp(), childpid1);
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
