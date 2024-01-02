#ifndef LIBS_SEM_H
#define LIBS_SEM_H

#include <sys/sem.h>

#include "util.h"
#include "console.h"

union semun {
    int val;                // value for SETVAL
    struct semid_ds* buf;   // buffer for IPC_STAT, IPC_SET
    unsigned short* array;  // array for GETALL, SETALL
#if defined(__linux__)      // Linux specific part
    struct seminfo* __buf;  // buffer for IPC_INFO
#endif
};

int mksem(key_t key, int nsems, int semflg, const int *init);
void sem_sync(int semid, int sem_num);
void sem_buf(struct sembuf *sop, unsigned short sem_num, short sem_op, short sem_flg);
int sem_op(int semid, struct sembuf *sops, int nsops);
int rmsem(int semid);

#endif