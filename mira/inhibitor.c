#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>

#include "mira.h"

sig_atomic_t stopped = 0;

void sig_usr_handler() {
    printf("SIGUSR1\n");
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

    while (1) {
        // Logica di inizio
        int *val = attach_shmem(shmid);

        struct sembuf sops[2];

        sops[0].sem_flg = IPC_NOWAIT;
        sops[0].sem_num = INH_ON;
        sops[0].sem_op = -1;
        if (semop(semid, sops, 1) == -1) {
            // è giusto che succeda se all'inizio l'inibitore gia' parte attivato
        }

        printf("Inibitore in funzione.\n");

        // Logica di mezzo
        while (!stopped) {

            sops[0].sem_flg = 0;
            sops[0].sem_num = INHIBITOR;
            sops[0].sem_op = -1;

            if (semop(semid, sops, 1) == -1) {
                printf("Inhibitor: mid (0).\n");
                break;
            }

            *val = *val - 1;

            if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
                printf("Error masking (1).\n");
                break;
            }

            sops[0].sem_flg = 0;
            sops[0].sem_num = MASTER;
            sops[0].sem_op = +1;

            if (semop(semid, sops, 1) == -1) {
                printf("Inhibitor: mid (1).\n");
                break;
            }

            if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) < 0) {
                printf("Error unmasking (2).\n");
                break;
            }

            if (stopped) {
                sops[0].sem_flg = 0;
                sops[0].sem_num = INH_ON;
                sops[0].sem_op = +1;
                if (semop(semid, sops, 1) == -1) {
                    printf("Inhibitor: mid (2).\n");
                    break;
                }
            }

            sops[0].sem_flg = 0;
            sops[0].sem_num = ATOM;
            sops[0].sem_op = +1;
            if (semop(semid, sops, 1) == -1) {
                printf("Inhibitor: mid (3).\n");
                break;
            }
        }
        // Logica di fine


        detach_shmem();

        while (stopped)
            ;
    }

}