#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define IPC_DIRECTORY ".so_ipc"

struct Ipc {
    int semid;
    pid_t master;
};

#endif
