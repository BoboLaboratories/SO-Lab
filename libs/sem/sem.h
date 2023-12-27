#ifndef LIBS_SEM_H
#define LIBS_SEM_H

#include <sys/sem.h>

#include "../console.h"
#include "../util/util.h"

union semun {
    int val;                // value for SETVAL
    struct semid_ds* buf;   // buffer for IPC_STAT, IPC_SET
    unsigned short* array;  // array for GETALL, SETALL
#if defined(__linux__)      // Linux specific part
    struct seminfo* __buf;  // buffer for IPC_INFO
#endif
};

#define SEM_SYNC            0
#define SEM_ATOM            1
#define SEM_MASTER          2
#define SEM_INHIBITOR       3
#define SEM_INHIBITOR_ON    4

#define SEM_COUNT           5

void sem_set(int semid);
void sem_sync();
void sem_buf(struct sembuf *sop, short sem_op, unsigned short sem_num, short sem_flg);
void sem_op(struct sembuf *sops, int nsops);

#endif