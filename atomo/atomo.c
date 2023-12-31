#include <stdlib.h>

#include "../model/model.h"
#include "../lib/util/util.h"
#include "../lib/shmem/shmem.h"
#include "../lib/sem/sem.h"

sig_atomic_t *interrupted = 0;
struct Model *model;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();


    // =========================================
    //          Setup shared memory
    // =========================================

    if (parse_int(argv[1], &model->res->shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if ((model->res->shmaddr = shmem_attach(model->res->shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(model->res->shmaddr);

    int atomic_number;
    parse_int(argv[2], &atomic_number);


    sem_sync(model->ipc->semid, SEM_SYNC);
//    print(D, "New atom just spawned with atomic number %d\n", atomic_number);

    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
    // TODO: cleanup lifo
}