#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

enum Component {
    MASTER=0,
    ATOMO=1,
    ATTIVATORE=2,
    ALIMENTATORE=3,
    INIBITORE=4
};

#define CTL  0
#define MAIN 1

struct IpcRes {
    int component;

    int shmid[2];
    void *adrr[2];
};

struct Ctl {
    key_t main_shmid;
};

struct ShmemCtl {
    key_t sem;
    pid_t *top_inactive;
    pid_t *top_active;
};

#define ATOMS_SHMID     ctl_shmem->main_shmid

#define ATOMS_SEMID     (((struct Shmem_ctl) shmem)->sem)
#define BOTTOM_INACTIVE (shmem + sizeof(struct Shmem_ctl))
#define TOP_INACTIVE    (*((struct Shmem_ctl) shmem)->top_inactive)
#define TOP_ACTIVE

void init_ipc(struct IpcRes *r, int component);
void attach_shmem(int id);
void free_ipc();

int errno_fail(char *format, int line, char *file, ...);
int fail(char *format, int line, char *file, ...);

#endif