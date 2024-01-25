#include <sys/shm.h>

#include "lib/shmem.h"
#include "lib/util.h"
#include "lib/console.h"


int shmem_create(key_t key, size_t size, int shmflg) {
    int shmid;

    if ((shmid = shmget(key, size, shmflg)) == -1) {
        print(E, "Could not create shared memory.\n");
    }

    return shmid;
}

void *shmem_attach(int shmid) {
    void *shmaddr;

    if ((shmaddr = shmat(shmid, NULL, 0)) == (void *) -1) {
        print(E, "Could not attach shared memory (%d).\n", shmid);
    }

    return shmaddr;
}

int shmem_detach(void *shmaddr) {
    int ret;

    if ((ret = shmdt(shmaddr)) == -1) {
        print(E, "Could not detach shared memory (%p).\n", shmaddr);
    }

    return ret;
}

int shmem_rmark(int shmid) {
    int ret;

    if ((ret = shmctl(shmid, IPC_RMID, NULL)) == -1) {
        print(E, "Could not mark shared memory for removal (%d).\n", shmid);
    }

    return ret;
}