#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define SEM_SYNC            0
#define SEM_ATOM            1
#define SEM_LIFO            2
#define SEM_MASTER          3
#define SEM_INHIBITOR       4
#define SEM_INHIBITOR_ON    5

#define SEM_COUNT           6

struct Ipc {
    int semid;
    pid_t master;
};

#endif
