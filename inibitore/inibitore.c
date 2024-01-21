#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/util.h"
#include "lib/lifo.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

int running();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();
    sig_handle(NULL, SIGTERM);


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

    sem_sync(model->ipc->semid, SEM_SYNC);

    while (running()) {
        struct sembuf sops;
        sem_buf(&sops, SEM_INIBITORE, -1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        mask(SIGTERM);
        long curr_energy = min(model->stats->curr_energy, ENERGY_DEMAND + ENERGY_EXPLODE_THRESHOLD - 1);

        long inhibited_energy = model->stats->curr_energy - curr_energy;
        model->stats->curr_energy = curr_energy;
        model->stats->inhibited_energy += inhibited_energy;

        pid_t pid;
        lifo_pop(model->lifo, &pid);
        kill(pid, SIGWAST);

        sem_buf(&sops, SEM_MASTER, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        sem_buf(&sops, SEM_ATTIVATORE, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        unmask(SIGTERM);
    }

    exit(EXIT_SUCCESS);
}

int running() {
    return sig_reset(sig != SIGTERM);
}

void cleanup() {
    if (model != NULL) {
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}