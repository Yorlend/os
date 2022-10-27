#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
		sleep(1);
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
		sleep(1);
		printf("\nChild 2:\n\tPID:\t%d\n\tPPID:\t%d\n\tPGRP:\t%d\n",
			getpid(), getppid(), getpgrp());
		return EXIT_SUCCESS;
	}

	printf("\nParent:\n\tPID:\t%d\n\tPGRP:\t%d\n\tChild:\t%d\n",
			getpid(), getpgrp(), childpid1);
}
