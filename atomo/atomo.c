#ifndef ATOMO
#define ATOMO
#endif

#include <stdlib.h>

#include "../model/model.h"
#include "../libs/util/util.h"
#include "../libs/shmem/shmem.h"

sig_atomic_t *interrupted = 0;
struct Model *model;
/*
    [0] = executable
    [1] = shmid
    [2] = atomic number
    [3] = semid, if semaphore synchronization is needed; NULL, otherwise
    [4] = NULL, if semaphore synchronization is needed; not present, otherwise
 */
int main(int argc, char *argv[]) {
    model->lifo = NULL;

    int shmid;
    if (parse_int(argv[1], &shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    void *shmaddr;
    if ((shmaddr = shmem_attach(shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    init_model(shmaddr);

    model->lifo;
}