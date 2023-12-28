#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define TMP_FILE ".so_ipc"

struct Ipc {
    int semid;
    pid_t master;
};

#endif
