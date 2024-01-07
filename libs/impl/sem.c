#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lib/sem.h"

struct Init {
    int sem_num;
    int val;
};

static int sem_set(int semid, int sem_num, int init) {
    union semun se;
    se.val = init;
    return semctl(semid, sem_num, SETVAL, se);
}

int mksem(key_t key, int nsems, int semflg, const int *init) {
    int semid = semget(key, nsems, semflg);
    int res = 0;

    if (semid != -1) {
        if (nsems == 1) {
            res = sem_set(semid, 0, *init);
        } else {
            struct Init *tmp = NULL;
            union semun se;
            se.array = malloc(nsems * sizeof(unsigned short));
            int tot_tmp = 0;

            for (int i = 0; i < nsems; i++) {
                if (init[i] <= USHRT_MAX) {
                    se.array[i] = init[i];
                } else {
                    tmp = reallocarray(tmp, ++tot_tmp, sizeof(struct Init));
                    tmp[tot_tmp - 1].val = init[i];
                    tmp[tot_tmp - 1].sem_num = i;
                    se.array[i] = 0;
                }
            }

            if (tot_tmp < nsems) {
                res = semctl(semid, 0, SETALL, se);
            }
            free(se.array);

            if (tmp != NULL) {
                for (int i = 0; res != -1 && i < tot_tmp; i++) {
                    res = sem_set(semid, tmp[i].sem_num, tmp[i].val);
                }
                free(tmp);
            }
        }

        if (res == -1) {
            rmsem(semid);
        }
    }

    return res == -1 ? -1 : semid;
}

void sem_sync(int semid, int sem_num) {
    struct sembuf sops;
    sem_buf(&sops, sem_num, -1, IPC_NOWAIT);
    if (sem_op(semid, &sops, 1) == -1) {
        if (errno == EAGAIN) {
            return;
        }
    }

    sem_buf(&sops, sem_num, 0, 0);
    sem_op(semid, &sops, 1);
}

void sem_buf(struct sembuf *sop, unsigned short sem_num, short sem_op, short sem_flg) {
    sop->sem_num = sem_num;
    sop->sem_flg = sem_flg;
    sop->sem_op = sem_op;
}
/*
int sem_acquire(int semid, unsigned short sem_num, short sem_flg) {
    struct sembuf sops;
    sem_buf(&sops, sem_num, 1, sem_flg);
    return sem_op(semid, &sops, 1);
}

int sem_release(int semid, unsigned short sem_num, short sem_flg) {
    struct sembuf sops;
    sem_buf(&sops, sem_num, -1, sem_flg);
    return sem_op(semid, &sops, 1);
}
*/
int sem_op(int semid, struct sembuf *sops, int nsops) {
    return semop(semid, sops, nsops);
}

int rmsem(int semid) {
    return semctl(semid, 0, IPC_RMID);
}

