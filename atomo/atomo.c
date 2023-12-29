#ifndef ATOMO
#define ATOMO
#endif

#include <stdlib.h>

#include "../model/model.h"
#include "../libs/util/util.h"
#include "../libs/shmem/shmem.h"
#include "../libs/sem/sem.h"

sig_atomic_t *interrupted = 0;
struct Model *model;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //          Setup shared memory
    // =========================================
    int shmid;
    if (parse_int(argv[1], &shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    void *shmaddr;
    if ((shmaddr = shmem_attach(shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(shmaddr);

    int atomic_number;
    parse_int(argv[2], &atomic_number);


    sem_sync();
//    print(D, "New atom just spawned with atomic number %d\n", atomic_number);

    exit(EXIT_SUCCESS);
}