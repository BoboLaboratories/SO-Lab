#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <errno.h>

#include "mira.h"

int main(int argc, char *argv[]) {
    int semid, shmid;

    if (parse_int(argv[1], &semid) == -1) {
        printf("Could not parse semid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (parse_int(argv[2], &shmid) == -1) {
        printf("Could not parse shmid (%s).\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    int *val = attach_shmem(shmid);

    while (1) {
        struct sembuf sops[2];

        sops[0].sem_flg = 0;
        sops[0].sem_num = ATOM;
        sops[0].sem_op = -1;

        sops[1].sem_flg = 0;
        sops[1].sem_num = MASTER;
        sops[1].sem_op = -1;

        if (semop(semid, sops, 2) == -1) {
            printf("Atom: start.\n");
            break;
        }

        *val = *val + 1;

        sops[0].sem_flg = IPC_NOWAIT;
        sops[0].sem_num = INH_ON;
        sops[0].sem_op = 0;

        if (semop(semid, sops, 1) == -1) {
            if (errno == EAGAIN) {
                // inibitore spento
                sops[0].sem_flg = 0;
                sops[0].sem_num = MASTER;
                sops[0].sem_op = +1;

                if (semop(semid, sops, 1) == -1) {
                    printf("Atom: end (1).\n");
                    break;
                }

                sops[0].sem_flg = 0;
                sops[0].sem_num = ATOM;
                sops[0].sem_op = +1;

                if (semop(semid, sops, 1) == -1) {
                    printf("Atom: end (2).\n");
                    break;
                }
            }
        } else {
            // inibitore acceso
            sops[0].sem_flg = 0;
            sops[0].sem_num = INHIBITOR;
            sops[0].sem_op = +1;

            if (semop(semid, sops, 1) == -1) {
                printf("Atom: end (3).\n");
                break;
            }
        }
    }

    detach_shmem();
}