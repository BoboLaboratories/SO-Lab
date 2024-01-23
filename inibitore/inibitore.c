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

    struct sembuf sops;
    while (sig != SIGTERM) {
        sem_buf(&sops, SEM_INIBITORE, -1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            if (errno == EINTR && sig == SIGTERM) {
                break;
            }
        }

        sigprocmask(SIG_BLOCK, &mask, NULL);

        long curr_energy = min(model->stats->curr_energy, ENERGY_DEMAND + ENERGY_EXPLODE_THRESHOLD - 1);
        long inhibited_energy = model->stats->curr_energy - curr_energy;
        model->stats->inhibited_energy += inhibited_energy;
        model->stats->curr_energy = curr_energy;

        pid_t pid = -1;
        lifo_pop(model->lifo, &pid);
        if (kill(pid, SIGWAST) == -1) {
            print(E, "Error wasting atom %d.\n", pid);
        }

        sem_buf(&sops, SEM_ATOM, +1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print (E, "Could not allow atom to update stats.\n");
        }

        sem_buf(&sops, SEM_ATOM, 0, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print(E, "Could not wait for atom to update stats.\n");
        }

        sem_end_activation(model->ipc->semid);

        sigprocmask(SIG_UNBLOCK, &mask, NULL);
    }

    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model != NULL) {
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}