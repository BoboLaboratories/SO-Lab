#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "mira.h"
#include "../libs/console.h"

int main() {
    int semid = semget(IPC_PRIVATE, 4, S_IWUSR | S_IRUSR);
    if (semid == -1) {
        printf("Could not get semaphore.\n");
        exit(EXIT_FAILURE);
    }

    union semun se;
    se.val = 1;
    semctl(semid, ATOM, SETVAL, se);
    se.val = 0;
    semctl(semid, INHIBITOR, SETVAL, se);
    se.val = 1;
    semctl(semid, MASTER, SETVAL, se);
    se.val = 0;
    semctl(semid, INH_ON, SETVAL, se);

    int shmid = shmget(IPC_PRIVATE, sizeof(int), 0666 | IPC_CREAT);
    if (shmid == 1) {
        printf("Could not get shared memory.\n");
        exit(EXIT_FAILURE);
    }

    int *val = attach_shmem(shmid);

    char *buf;
    char **argv;
    prargs("atom", &argv, &buf, 2, ITC_SIZE);
    sprintf(argv[1], "%d", semid);
    sprintf(argv[2], "%d", shmid);
    for (int i = 0; i < 100; i++) {
        if (fork_execve(argv) == -1) {
            printf("Could not fork %s.\n", argv[0]);
        }
    }

    argv[0] = "inhibitor";
    if (fork_execve(argv) == -1) {
        printf("Could not fork %s.\n", argv[0]);
    }

    int i = 0;
    while (1) {
        sleep(1);

        struct sembuf sops;

        sops.sem_flg = 0;
        sops.sem_num = MASTER;
        sops.sem_op = -1;

        if (semop(semid, &sops, 1) == -1) {
            printf("Master: start.\n");
            break;
        }

        printf("val=%d \t\t%d\n", *val, i);
        i++;

        sops.sem_flg = 0;
        sops.sem_num = MASTER;
        sops.sem_op = +1;

        if(semop(semid, &sops, 1) == -1) {
            printf("Master: end.\n");
            break;
        }
    }

    detach_shmem();

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        printf("Could not request shared memory removal (shmid: %d)\n", shmid);
        ERRNO_PRINT;
    }

    if (semctl(semid, 0, IPC_RMID) == -1) {
        printf("Could not remove semaphores.\n");
        ERRNO_PRINT;
    }
}
