#include <stdlib.h>

#include "shmem.h"

int shmem_create(key_t key, size_t size, int shmflg) {
    int shmid = shmget(key, size, shmflg);
    if (shmid == -1) {
        print(E, "Could not create shared memory.\n");
    }
#if defined(DEBUG) || defined(D_SHMEM)
    else {
        printf(D "Created shared memory (%d).\n", shmid);
    }
#endif

    return shmid;
}

void *shmem_attach(int shmid) {
    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *) -1) {
        print(E, "Could not attach shared memory.\n");
    }
#if defined(DEBUG) || defined(D_SHMEM)
    else {
        printf(D "Attached shared memory (%d, %p).\n", shmid, shmaddr);
    }
#endif

    return shmaddr;
}

int shmem_detach(void *shmaddr) {
    int ret = shmdt(shmaddr);
    if (ret == -1) {
        print(E, "Could not detach shared memory (%p).\n", shmaddr);
    }
#if defined(DEBUG) || defined(D_SHMEM)
    else {
        printf(D "Attached shared memory (%p).\n", shmaddr);
    }
#endif

    return ret;
}

int shmem_rmark(int shmid) {
    int ret = shmctl(shmid, IPC_RMID, NULL);
    if (ret == -1) {
        print(E, "Could not mark shared memory for removal (%d).\n", shmid);
    }
#if defined(DEBUG) || defined(D_SHMEM)
    else {
        printf(D "Marked shared memory for removal (%d).\n", shmid);
    }
#endif

    return ret;
}