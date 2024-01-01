#include <math.h>
#include <stdlib.h>

#include "../model/model.h"
#include "../lib/sem/sem.h"
#include "../lib/shmem/shmem.h"
#include "../lib/signal/signal.h"

sig_atomic_t interrupted = 0;
sig_atomic_t fission = 0;
struct Model *model;

static int atomic_number;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (parse_int(argv[1], &model->res->shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (parse_int(argv[2], &atomic_number) == -1) {
        print(E, "Could not parse atomic number (%s).\n", argv[2]);
        exit(EXIT_FAILURE);
    }


    init();

    // =========================================
    //          Setup shared memory
    // =========================================
    if ((model->res->shmaddr = shmem_attach(model->res->shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(model->res->shmaddr);


    // Lifo here
    pid_t pid = getpid();
    lifo_push(model->lifo, &pid);

    set_sighandler(SIGUSR1, ...);

    sem_sync(model->ipc->semid, SEM_SYNC);

    srand(getpid());
    // Funzione 1
    int child_atomic_number = rand_between(1, atomic_number - 1);
    atomic_number = atomic_number - child_atomic_number;

    // Funzione 2
    child_atomic_number = floor((double) atomic_number / 2);
    atomic_number = atomic_number - child_atomic_number;

    while (pause() == -1 && !interrupted) {
        if (fission == 1) {
        
        }
    }


    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
    // TODO: cleanup lifo
}

void sigterm_handler() {
    interrupted = 1;
}

void sigusr1_handler() {
    fission = 1;
}