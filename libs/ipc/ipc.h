#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

struct IpcRes {
    /* Control shared memory */
    int ctl_shmid;
    void *ctl_addr;

    /* Main shared memory */
    int main_shmid;
    void *main_shmem_addr;
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

#endif