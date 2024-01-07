#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/util.h"
#include "lib/lifo.h"
#include "lib/shmem.h"

void signal_handler(int signum);

struct Model *model;
sig_atomic_t interrupted = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
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

    sem_sync(model->ipc->semid, SEM_SYNC);

    while (!interrupted) {
        struct sembuf sops;
        sem_buf(&sops, SEM_INIBITORE, -1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        long curr_energy = min(model->stats->curr_energy, ENERGY_DEMAND + ENERGY_EXPLODE_THRESHOLD - 1);

        long inhibited_energy = model->stats->curr_energy - curr_energy;
        model->stats->curr_energy = curr_energy;
        model->stats->inhibited_energy += inhibited_energy;

        pid_t pid;
        lifo_pop(model->lifo, &pid);
        kill(pid, SIGWAST);

        sem_buf(&sops, SEM_MASTER, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        sem_buf(&sops, SEM_ATOM, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);
    }

    exit(EXIT_SUCCESS);
}

void signal_handler(int signum) {
    if (signum == SIGTERM) {
        interrupted = 1;
    }
}

void cleanup() {
    if (model != NULL) {
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}