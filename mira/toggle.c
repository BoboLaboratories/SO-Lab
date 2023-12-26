#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#include "mira.h"

int main(int argc, char *argv[]) {
    key_t key = ftok("ftok", 2);
    int semid = semget(key, 4, S_IWUSR | S_IRUSR);
    if (semid == -1) {
        printf("Could not get semaphore.\n");
        exit(EXIT_FAILURE);
    }

    struct sembuf sops;

    sops.sem_flg = 0;
    sops.sem_num = INH_ON;
    sops.sem_op = atoi(argv[1]);

    if (semop(semid, &sops, 1) == -1) {
        printf("Toggle failed.\n");
    } else {
        printf("Toggle success.\n");
    }
}