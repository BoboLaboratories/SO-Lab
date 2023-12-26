#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>

#include "mira.h"

sig_atomic_t stopped = 0;

void sig_usr_handler() {
    stopped = stopped == 1 ? 0 : 1;
}

int main(int argc, char *argv[]) {
    sigset_t orig_mask, mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    int semid, shmid;
    if (parse_int(argv[1], &semid) == -1) {
        printf("Could not parse semid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (parse_int(argv[2], &shmid) == -1) {
        printf("Could not parse shmid (%s).\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_usr_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        printf("Could not set SIGTERM handler.\n");
        exit(EXIT_FAILURE);
    }

    int *val = attach_shmem(shmid);

    while (1) {
        struct sembuf sops;

        sops.sem_flg = 0;
        sops.sem_num = INHIBITOR;
        sops.sem_op = -1;

        if (semop(semid, &sops, 1) == -1) {
            printf("Inhibitor (1).\n");
            break;
        }

        *val = *val - 1;

        sops.sem_flg = 0;
        sops.sem_num = MASTER;
        sops.sem_op = +1;

        if (semop(semid, &sops, 1) == -1) {
            printf("Inhibitor (2).\n");
            break;
        }

        sops.sem_flg = 0;
        sops.sem_num = ATOM;
        sops.sem_op = +1;

        if (semop(semid, &sops, 1) == -1) {
            printf("Inhibitor (3).\n");
            break;
        }
    }

}