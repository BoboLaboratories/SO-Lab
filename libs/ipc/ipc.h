#ifndef IPC_H
#define IPC_H

#include <sys/types.h>
#include "../lifo/lifo.h"

#define IPC_DIRECTORY ".ipc"
#define FIFO_PATHNAME IPC_DIRECTORY "/fifo"

enum Component {
    MASTER,
    ATOMO,
    ATTIVATORE,
    ALIMENTATORE,
    INIBITORE
};

enum Status {
    RUNNING,
    TIMEOUT,
    EXPLODE,
    BLACKOUT,
    MELTDOWN
};

struct IpcRes {
    enum Component component;
    int shmid;
    void *addr;
    int fifo_fd;
};

struct Model {
    struct Config *config;
    struct Stats {
        enum Status status;
        long energy;
        int n_atoms;
        int n_wastes;
    } *stats;
    struct Lifo *lifo;
};

union semun {
    // value for SETVAL
    int val;
    // buffer for IPC_STAT, IPC_SET
    struct semid_ds* buf;
    // array for GETALL, SETALL
    unsigned short* array;
    // Linux specific part
#if defined(__linux__)
    // buffer for IPC_INFO
    struct seminfo* __buf;
#endif
};

void init_ipc(struct IpcRes **res, enum Component component);
void attach_model();
void attach_shmem();
void free_ipc();

// fifo
void open_fifo(int flags);
void close_fifo();

// semaphores
void sem_sync(int semid);

// failures
int errno_fail(char *format, int line, char *file, ...);
int fail(char *format, int line, char *file, ...);

#endif