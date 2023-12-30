#include "sem.h"
#include "errno.h"

void sem_sync() {
    // TODO what is a signal interrupts? D:

    // TODO error handling

    struct sembuf sops;
    sem_buf(&sops, -1, SEM_SYNC, IPC_NOWAIT);
    if (sem_op(&sops, 1) == -1) {
        if (errno == EAGAIN) {
            return;
        }
    }

    sem_buf(&sops, 0, SEM_SYNC, 0);
    sem_op(&sops, 1);
}

void sem_buf(struct sembuf *sop, short sem_op, unsigned short sem_num, short sem_flg) {
    sop->sem_num = sem_num;
    sop->sem_flg = sem_flg;
    sop->sem_op = sem_op;
}

int sem_op(struct sembuf *sops, int nsops) {
    extern struct Model *model;
    return semop(model->ipc->semid, sops, nsops);
}