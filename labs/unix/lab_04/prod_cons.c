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

struct shared_buffer
{
    char next;
    char* cons_addr;
    char* prod_addr;
    char* buffer;
};

void consume(int shmfd, int semfd);
void produce(int shmfd, int semfd);

void sig_handler(int signum);

const int perms = S_IRWXU | S_IRWXG | S_IRWXO;

const int capacity = 15;
const int sem_count = 3;
const int prod_count = 3;
const int cons_count = 3;

const int sem_full_id = 0;
const int sem_bin_id = 1;
const int sem_empty_id = 2;

const int total_shared_mem = capacity + sizeof(struct shared_buffer);

struct sembuf cons_lock[2] = { { sem_full_id, -1, SEM_UNDO }, { sem_bin_id, -1, SEM_UNDO } };
struct sembuf cons_unlock[2] = { { sem_bin_id, 1, 0}, { sem_empty_id, 1, 0 } };
struct sembuf prod_lock[2] = { { sem_bin_id, -1, SEM_UNDO }, { sem_empty_id, -1, SEM_UNDO } };
struct sembuf prod_unlock[2] = { { sem_full_id, 1, 0 }, { sem_bin_id, 1, 0 } };

int main(void)
{
    pid_t childpid;
    int shmfd, semfd;
    struct shared_buffer *buff;

    shmfd = shmget(100, total_shared_mem, IPC_CREAT | perms);
    if (shmfd == -1)
    {
        perror("shmget");
        exit(1);
    }

    buff = shmat(shmfd, 0, 0);
    if (buff == (struct shared_buffer*)-1)
    {
        perror("shmat");
        exit(1);
    }

    buff->buffer = (char*)(buff + sizeof(struct shared_buffer));
    buff->next = 'a';
    buff->cons_addr = buff->buffer;
    buff->prod_addr = buff->buffer;
    if (shmdt(buff) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    semfd = semget(100, sem_count, IPC_CREAT | perms);
    if (semfd == -1)
    {
        perror("semget");
        exit(1);
    }

    if (semctl(semfd, sem_full_id, SETVAL, 0) == -1 || semctl(semfd, sem_bin_id, SETVAL, 1) == -1 || semctl(semfd, sem_empty_id, SETVAL, capacity) == -1)
    {
        perror("semctl");
        exit(1);
    }

    for (int i = 0; i < cons_count; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(1);
        }

        if (childpid == 0)
        {
            consume(shmfd, semfd);
            exit(0);
        }
    }

    for (int i = 0; i < prod_count; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(1);
        }

        if (childpid == 0)
        {
            produce(shmfd, semfd);
            exit(0);
        }
    }

    signal(SIGTERM, SIG_IGN);

    for (int i = 0; i < prod_count + cons_count; i++)
        wait(0);
    
    printf("Программа завершена.\n");
    if (semctl(semfd, 0, IPC_RMID) == -1)
        perror("semctl: IPC_RMID");
    
    if (shmctl(shmfd, IPC_RMID, 0) == -1)
        perror("shmctl: IPC_RMID");

    return 0;
}

void consume(int shmfd, int semfd)
{
    char value;
    struct shared_buffer *buff;
    
    buff = shmat(shmfd, 0, 0);
    if (buff == (struct shared_buffer*)-1)
    {
        perror("cons_shmat");
        exit(1);
    }

    while (1)
    {
        usleep(200000 + rand() % 200000);

        if (semop(semfd, cons_lock, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        value = *(buff->cons_addr++);
        if (buff->cons_addr == buff->buffer + capacity)
            buff->cons_addr = buff->buffer;

        if (semop(semfd, cons_unlock, 2) == -1)
        {
            perror("semop");
            exit(1);
        }
        
        printf("consumer pid: %d: %c\n", getpid(), value);

        if (value == 'z')
            killpg(getpgrp(), SIGTERM);
    }
}

void produce(int shmfd, int semfd)
{
    struct shared_buffer *buff;

    buff = shmat(shmfd, 0, 0);
    if (buff == (struct shared_buffer*)-1)
    {
        perror("prod_shmat");
        exit(1);
    }

    while (1)
    {
        usleep(200000 + rand() % 200000);

        if (semop(semfd, prod_lock, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        *(buff->prod_addr++) = buff->next++;
        if (buff->prod_addr == buff->buffer + capacity)
            buff->prod_addr = buff->buffer;
        printf("producer pid: %d\n", getpid());

        if (semop(semfd, prod_unlock, 2) == -1)
        {
            perror("semop");
            exit(1);
        }
    }
}
