#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sem.h>

#include "mira.h"

int main(int argc, char *argv[]) {
    key_t key = ftok("ftok", 2);
    int semid = semget(key, 4, S_IWUSR | S_IRUSR);
    if (semid == -1) {
        printf("Could not get semaphore.\n");
        exit(EXIT_FAILURE);
    }

    struct sembuf sops[2];

    sops[0].sem_flg = IPC_NOWAIT;
    sops[0].sem_num = INH_ON;
    sops[0].sem_op = 0;

    sops[1].sem_flg = 0;
    sops[1].sem_num = INH_ON;
    sops[1].sem_op = +1;

    if (semop(semid, sops, 2) == -1) {
        if (errno == EAGAIN) {
            sops[0].sem_flg = IPC_NOWAIT;
            sops[0].sem_num = INH_ON;
            sops[0].sem_op = -1;
            if (semop(semid, sops, 1) == -1) {
                printf("Toggle failed.\n");
            } else {
                printf("Toggle: ON\n");
            }
        }
    } else {
        printf("Toggle: OFF\n");
    }
}