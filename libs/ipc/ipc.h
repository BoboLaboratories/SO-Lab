#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

enum Component {
    MASTER,
    ATOMO,
    ATTIVATORE,
    ALIMENTATORE,
    INIBITORE
};

enum Shmem {
    CTL,
    MAIN
};

struct IpcRes {
    enum Component component;
    int shmid[2];
    void *addr[2];
};

struct Model {
    struct Config *config;
    struct Control {
        key_t *shmid;
    } *ctl;
    struct Atoms {
        key_t *sem;
        pid_t *top_inactive;
        pid_t *top_active;
    } *atoms;
};

void init_ipc(struct IpcRes *r, int component);
void attach_model();
void attach_shmem(enum Shmem which);
void free_ipc();

int errno_fail(char *format, int line, char *file, ...);
int fail(char *format, int line, char *file, ...);

#endif