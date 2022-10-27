#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void status_handler(int);

int main()
{
    pid_t childpid_1;
    if ((childpid_1 = fork()) == -1)
    {
        perror("Can't fork\n");
        return EXIT_FAILURE;
    }
    if (childpid_1 == 0)
    {
        if(execl("./programs/stack", "stack", NULL) == -1)
        {
            perror("Can't execute program");
            return EXIT_FAILURE;
        }
    }

    pid_t childpid_2;
    if ((childpid_2 = fork()) == -1)
    {
        perror("Can't fork\n");
        return EXIT_FAILURE;
    }
    if (childpid_2 == 0)
    {
        if(execl("./programs/queue", "queue", NULL) == -1)
        {
            perror("Can't execute program");
            return EXIT_FAILURE;
        }
    }

    int status;
    pid_t child_pid;

    child_pid = wait(&status);
    printf("\nstatus: %d\tchild_pid: %d\n", status, child_pid);
	status_handler(status);

    child_pid = wait(&status);
    printf("\nstatus: %d\tchild_pid: %d\n", status, child_pid);
	status_handler(status);

    printf("\nParent.\n");
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
