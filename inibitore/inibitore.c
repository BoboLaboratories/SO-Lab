#include <stdlib.h>
#include <errno.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/util.h"
#include "lib/lifo.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

int main(int argc, char *argv[]) {
#ifdef D_PID
    print(D, "Inibitore: %d\n", getpid());
#endif
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sig_setup(&mask, NULL, -1);


    // =========================================
    //           Setup shared memory
    // =========================================
    if (parse_int(argv[1], &model->res->shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if ((model->res->shmaddr = shmem_attach(model->res->shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(model->res->shmaddr);


    // =========================================
    //         Sync with master process
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //                Main logic
    // =========================================
    struct sembuf sops;
    while (1) {
        // wait for atoms to give us work
        sem_buf(&sops, SEM_INIBITORE, -1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print(E, "Could not wait for more jobs.\n");
            break;
        }

        mask(SIGTERM);

        // inhibit produced energy
        long curr_energy = min(model->stats->curr_energy, ENERGY_DEMAND + ENERGY_EXPLODE_THRESHOLD - 1);
        long inhibited_energy = model->stats->curr_energy - curr_energy;
        model->stats->inhibited_energy += inhibited_energy;
        model->stats->curr_energy = curr_energy;

        // waste an atom
        pid_t pid = -1;
        lifo_pop(model->lifo, &pid);
        if (kill(pid, SIGWAST) == -1) {
            print(E, "Error wasting atom %d.\n", pid);
        }

        // allow an atom to update waste stats
        sem_buf(&sops, SEM_ATOM, +1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print (E, "Could not allow atom to update stats.\n");
        }

        // wait for the above-mentioned atom to perform said update
        sem_buf(&sops, SEM_ATOM, 0, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print(E, "Could not wait for atom to update stats.\n");
        }

        end_activation_cycle();

        unmask(SIGTERM);
    }

    exit(EXIT_FAILURE);
}

void cleanup() {
    if (model != NULL) {
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}