#include "sem.h"

static int semid;

void sem_set(int _semid) {
    semid = semid;
}

void sem_sync() {
    // TODO what is a signal interrupts? D:
    // what if it doesn't

    struct sembuf sops;

    // signal we are ready
    sem_buf(&sops, -1, SEM_SYNC, 0);
    sem_op(&sops, 1);
    // TODO what if it fails?
    // what if it doesn't

    // wait for everyone to be ready
    sem_buf(&sops, 0, SEM_SYNC, 0);
    sem_op(&sops, 1);
}

void sem_buf(struct sembuf *sop, short sem_op, unsigned short sem_num, short sem_flg) {
    sop->sem_num = sem_num;
    sop->sem_flg = sem_flg;
    sop->sem_op = sem_op;
}

void sem_op(struct sembuf *sops, int nsops) {
    semop(semid, sops, nsops);
}