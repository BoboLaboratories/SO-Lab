#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define IPC_DIRECTORY ".ipc"
#define FIFO_PATHNAME IPC_DIRECTORY "/fifo"

enum Component {
    MASTER,
    ATOMO,
    ATTIVATORE,
    ALIMENTATORE,
    INIBITORE
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
        int energy_explode_threshold;
    } *stats;
};

void init_ipc(struct IpcRes *r, enum Component component);
void attach_model();
void attach_shmem();
void free_ipc();

// fifo
void open_fifo(int flags);
void close_fifo();

// failures
int errno_fail(char *format, int line, char *file, ...);
int fail(char *format, int line, char *file, ...);

#endif