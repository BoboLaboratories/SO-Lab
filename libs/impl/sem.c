#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lib/sem.h"
#include "lib/console.h"

// used to init a semaphore array when its value exceeds USHRT_MAX
struct Init {
    int sem_num;
    int val;
};

static int sem_set(int semid, int sem_num, int val) {
    union semun se;
    se.val = val;
    return semctl(semid, sem_num, SETVAL, se);
}

int sem_create(key_t key, int nsems, int semflg, const int *init) {
    int res = 0;
    int semid;

    if ((semid = semget(key, nsems, semflg)) == -1) {
       print(E, "Could not get a new semaphore set.\n");
       return -1;
    }


    if (nsems == 1) {
        // if the semaphore set is made up of a single
        // semaphore perform a simple set operation
        res = sem_set(semid, 0, *init);
    } else {
        union semun se;
        int tot_tmp = 0;
        struct Init *tmp = NULL;
        se.array = malloc(nsems * sizeof(unsigned short));

        // for each semaphore, if the value is less or equals to USHRT_MAX
        // use the semun.array (short[]) to initialize it, otherwise keep track
        // of the need to initialize it using semun.val (int)
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

        // if any semaphore in the semaphore set is to be initialized using
        // semun.array proceed and free the array as it's no longer needed
        if (tot_tmp < nsems) {
            if ((res = semctl(semid, 0, SETALL, se)) == -1) {
                print(E, "Could not initialize semaphore set (%d).\n", semid);
            }
        }
        free(se.array);

        // keep initializing values that exceeds USHRT_MAX
        // until there are more and no error is encountered
        if (tmp != NULL) {
            for (int i = 0; res != -1 && i < tot_tmp; i++) {
                if ((res = sem_set(semid, tmp[i].sem_num, tmp[i].val)) == - 1)  {
                    print(E, "Could not initialize semaphore %d in set %d to %d.\n", tmp[i].sem_num, semid, tmp[i].val);
                }
            }
            free(tmp);
        }
    }

    // if any error was encountered, automatically delete the semaphore set
    if (res == -1) {
        sem_delete(semid);
    }

    return res == -1 ? -1 : semid;
}

int sem_sync(int semid, int sem_num) {
    struct sembuf sops;

    // try to acquire the semaphore for synchronizing with other processes
    sem_buf(&sops, sem_num, -1, IPC_NOWAIT);
    if (sem_op(semid, &sops, 1) == -1) {
        if (errno != EAGAIN) {
            print(E, "Error synchronizing with other processes.\n");
            return -1;
        }
    }

    // wait for any other process that's synchronizing on the same semaphore
    sem_buf(&sops, sem_num, 0, 0);
    sem_op(semid, &sops, 1);

    return 0;
}

// one-liner for initializing sembuf struct
void sem_buf(struct sembuf *sop, unsigned short sem_num, short sem_op, short sem_flg) {
    sop->sem_num = sem_num;
    sop->sem_flg = sem_flg;
    sop->sem_op = sem_op;
}

int sem_op(int semid, struct sembuf *sops, int nsops) {
    return semop(semid, sops, nsops);
}

int sem_delete(int semid) {
    int ret;

    if ((ret = semctl(semid, 0, IPC_RMID)) == -1) {
        print(E, "Could not delete semaphore set (%d).\n", semid);
    }

    return ret;
}