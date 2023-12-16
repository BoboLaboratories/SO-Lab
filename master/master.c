#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "../libs/ipc/ipc.h"

struct Ctl_mem *ctl;

int main() {
    struct Ipc_res ipc_res;

    int ctl_shmid;
    if ((ctl_shmid = shmget(IPC_PRIVATE, sizeof(struct Ctl_mem), 0666 | IPC_CREAT)) == -1) {
        printf("Errore nella crazione della memoria\n");
        return EXIT_FAILURE;
    }

    ctl = (struct Ctl_mem *) shmat(ctl_shmid, NULL, 0);
    if (ctl == (void *) -1) {
        // TODO delete shared memory
        return EXIT_FAILURE;
    }

    int shmid;
    if ((shmid = shmget(IPC_PRIVATE, sizeof(struct Ctl_mem), 0666 | IPC_CREAT)) == -1) {
        // TODO detach shared memory
        // TODO delete shared memory
        return EXIT_FAILURE;
    }

}