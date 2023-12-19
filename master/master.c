#include <malloc.h>
#include <sys/shm.h>

#include "config.h"
#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/config/config.h"

void setup_ipc(struct IpcRes *res);
void setup_shmem(struct IpcRes *res, enum Shmem which, size_t size);

struct Model *model;

int main() {
    struct IpcRes res;

    setbuf(stdout, NULL); // TODO si vuole?
    setbuf(stderr, NULL); // TODO si vuole?

    setup_ipc(&res);
    attach_model();
    load_config();

    printf("%ld\n", N_ATOMI_INIT);

    free_ipc();
}

void setup_ipc(struct IpcRes *res) {
    init_ipc(res, MASTER);
    setup_shmem(res, CTL, sizeof(struct Control));
    setup_shmem(res, MAIN, sizeof(int));
}

void setup_shmem(struct IpcRes *res, enum Shmem which, size_t size) {
    res->shmid[which] = shmget(IPC_PRIVATE, size, 0666 | IPC_CREAT);
    if (res->shmid[which] == -1) {
        errno_fail("Could not create shared memory\n", F_INFO);
    }

    #ifdef DEBUG
        printf(D "Obtained shared memory from OS (shmid: %d)\n", res->shmid[which]);
    #endif

    attach_shmem(which);
}