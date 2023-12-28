#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

struct Ipc {
    int semid;
    pid_t master;
};

#endif
