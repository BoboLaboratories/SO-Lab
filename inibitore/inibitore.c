#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/ipc.h"
#include "lib/util.h"
#include "lib/lifo.h"
#include "lib/shmem.h"
#include "lib/console.h"

extern struct Model *model;
extern sig_atomic_t sig;

static int log;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <0:log off, 1:log on>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sig_setup(&mask, NULL, -1);
    sigprocmask(SIG_SETMASK, &mask, NULL);


    // =========================================
    //   Initialize process data and behaviour
    // =========================================
    init(argv[1]);


    // =========================================
    //            Argument parsing
    // =========================================
    if (parse_int(argv[2], &log) == -1) {
        print(E, "Could not parse log (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }


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
        if (log && inhibited_energy > 0) {
            print(I, "Inhibitor reducing energy by " YELLOW "%lu" RESET, inhibited_energy);
        }

        // waste an atom
        pid_t pid;
        if (lifo_pop(model->lifo, &pid) != -1) {
            if (kill(pid, SIGWAST) == -1) {
                print(E, "Error wasting atom %d.\n", pid);
            } else {
                model->stats->inhibited_atoms++;
                if (log) {
                    if (inhibited_energy > 0) {
                        printf(" and wasting atom " YELLOW "%d" RESET, pid);
                    } else {
                        print(I, "Inhibitor wasting atom " YELLOW "%d" RESET, pid);
                    }
                }
            }
        }
        if (log) {
            printf("\n");
        }

        // allow an atom to update waste simulation status
//        sem_buf(&sops, SEM_ATOM, +1, 0);
//        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
//            print (E, "Could not allow atom to update sim.\n");
//        }

        // wait for the above-mentioned atom to perform said update
//        sem_buf(&sops, SEM_ATOM, 0, 0);
//        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
//            print(E, "Could not wait for atom to update sim.\n");
//        }

//        end_activation_cycle();

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