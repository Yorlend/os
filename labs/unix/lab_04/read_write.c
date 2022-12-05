#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
const int can_read = 0;
const int can_write = 1;
const int active_read = 2;
const int wait_write = 3;

const int read_count = 4;
const int write_count = 3;

struct sembuf read_start[] = { {can_read, -1, SEM_UNDO}, {wait_write, 0, 0}, {active_read, 1, 0}, {can_read, 1, 0} };
struct sembuf read_end[] = { {active_read, -1, 0 }};

struct sembuf write_start[] = { {wait_write, 1, 0}, {active_read, 0, 0}, {can_write, -1, SEM_UNDO}, {can_read, -1, SEM_UNDO}, {wait_write, -1, SEM_UNDO} };
struct sembuf write_end[] = { {can_read, 1, 0}, {can_write, 1, 0} };

int start_read(int semfd);
int stop_read(int semfd);

int start_write(int semfd);
int stop_write(int semfd);

void read_work(int semfd, int shmfd);
void write_work(int semfd, int shmfd);

int main(void)
{
    int *counter;
    int shmfd, semfd;
    pid_t childpid;

    shmfd = shmget(100, sizeof(int), IPC_CREAT | perms);
    if (shmfd == -1)
    {
        perror("shmget");
        exit(1);
    }

    counter = shmat(shmfd, 0, 0);
    if (counter == (int*)-1)
    {
        perror("shmat");
        exit(1);
    }

    *counter = 0;

    if (shmdt(counter) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    semfd = semget(100, 4, IPC_CREAT | perms);

    if (semfd == -1)
    {
        perror("semget");
        exit(1);
    }

    if (semctl(semfd, can_read, SETVAL, 1) == -1 || semctl(semfd, can_write, SETVAL, 1) == -1 || semctl(semfd, active_read, SETVAL, 0) == -1 || semctl(semfd, wait_write, SETVAL, 0) == -1)
    {
        perror("semctl");
        exit(1);
    }

    for (int i = 0; i < read_count; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("fork");
            exit(1);
        }
        if (childpid == 0)
        {
            read_work(semfd, shmfd);
            exit(0);
        }
    }

    for (int i = 0; i < write_count; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("fork");
            exit(1);
        }
        if (childpid == 0)
        {
            write_work(semfd, shmfd);
            exit(0);
        }
    }

    for (int i = 0; i < read_count + write_count; i++)
    {
        wait(NULL);
    }

    if (semctl(semfd, 0, IPC_RMID) == -1)
    {
        perror("semctl");
        exit(1);
    }

    return 0;
}

int start_read(int semfd)
{
    return semop(semfd, read_start, 4);
}

int stop_read(int semfd)
{
    return semop(semfd, read_end, 1);
}

int start_write(int semfd)
{
    return semop(semfd, write_start, 5);
}

int stop_write(int semfd)
{
    return semop(semfd, write_end, 2);
}

void read_work(int semfd, int shmfd)
{
    int *counter = shmat(shmfd, 0, 0);
    if (counter == (int*)-1)
    {
        perror("shmat");
        exit(1);
    }

    while(1)
    {
        usleep(200000 + rand() % 200000);

        if (start_read(semfd) == -1)
        {
            perror("start_read");
            exit(1);
        }

        // start critical section
        printf("reader (pid %d): %d\n", getpid(), *counter);
        // end critical section

        if (stop_read(semfd) == -1)
        {
            perror("stop_read");
            exit(1);
        }
    }
}

void write_work(int semfd, int shmfd)
{
    int *counter = shmat(shmfd, 0, 0);
    if (counter == (int*)-1)
    {
        perror("shmat");
        exit(1);
    }

    while(1)
    {
        usleep(200000 + rand() % 200000);

        if (start_write(semfd) == -1)
        {
            perror("start_write");
            exit(1);
        }

        // start critical section
        (*counter)++;
        printf("writer (pid %d): \t\t%d\n", getpid(), *counter);
        // end critical section

        if (stop_write(semfd) == -1)
        {
            perror("stop_write");
            exit(1);
        }
    }
}
