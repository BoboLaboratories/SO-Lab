#include <stdlib.h>
#include <errno.h>

#ifdef FISSION_HALF
#include <math.h>
#endif

#include "model.h"
#include "lib/sem.h"
#include "lib/shmem.h"
#include "lib/sig.h"

void signal_handler(int signum);

void split(int *atomic_number, int *child_atomic_number);

struct Model *model;
sig_atomic_t sig;
sig_atomic_t interrupted = 0;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int child_atomic_number;
    int atomic_number;

    init();

    if (parse_int(argv[1], &model->res->shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (parse_int(argv[2], &atomic_number) == -1) {
        print(E, "Could not parse atomic number (%s).\n", argv[2]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //          Setup shared memory
    // =========================================
    if ((model->res->shmaddr = shmem_attach(model->res->shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }
    attach_model(model->res->shmaddr);

    set_sighandler(SIGACTV, &signal_handler);
    set_sighandler(SIGWAST, &signal_handler);

    sem_acquire(model->ipc->semid, SEM_MASTER, 0);
    model->stats->n_atoms++;
    sem_release(model->ipc->semid, SEM_MASTER, 0);

    sem_sync(model->ipc->semid, SEM_SYNC);

    pid_t pid = getpid();
    srand(pid);

    while (!interrupted) {
        // wait until no action is requested
        pause();
        // if fission was requested
        if (sig == SIGACTV) {
            // if this atom should become waste
            if (atomic_number < MIN_N_ATOMICO) {
                // TODO: Segnala all'alimentatore che può immettere un atomo (se l'inibitore è presente)
                sem_acquire(model->ipc->semid, SEM_MASTER, 0);
                model->stats->n_wastes++;
                model->stats->n_atoms--;
                sem_release(model->ipc->semid, SEM_MASTER, 0);
                // print(W, "%d(%d) became waste\n", pid, atomic_number);
                break;
            }

            // prepare child arguments and fork a new atom
            split(&atomic_number, &child_atomic_number);
            sprintf(argv[2], "%d", child_atomic_number);
            pid_t child_pid = fork_execve(argv);

            if (child_pid != -1) {
                // if fork was successful
                // produce energy and update stats
                long energy = (atomic_number * child_atomic_number) - max(atomic_number, child_atomic_number);
                // print(W, "%d(%d) %d(%d) = %ld.\n", pid, atomic_number, child_pid, child_atomic_number, energy);

                struct sembuf sops[2];
                sem_buf(&sops[0], SEM_MASTER, -1, 0);
                sem_buf(&sops[1], SEM_ATOM, -1, 0);

                sem_op(model->ipc->semid, sops, 2);
                // TODO: Error handle

                model->stats->curr_energy += energy;
                model->stats->n_fissions++;

                // put this atom and the new one on top of the lifo
                lifo_push(model->lifo, &pid);
                lifo_push(model->lifo, &child_pid);

                // wake up inhibitor, if activated, to inhibit the energy we just produced
                sem_buf(&sops[0], SEM_INHIBITOR_ON, 0, IPC_NOWAIT);
                sem_buf(&sops[1], SEM_INHIBITOR, 1, 0);
                if (sem_op(model->ipc->semid, sops, 2) == -1) {
                    if (errno == EAGAIN) {
                        // if inhibitor is deactivated, give control back to master process
                        sem_buf(&sops[0], SEM_MASTER, 1, 0);
                        sem_op(model->ipc->semid, &sops[0], 1);

                        // and let another atom perform its job
                        sem_buf(&sops[0], SEM_ATOM, 1, 0);
                        sem_op(model->ipc->semid, &sops[0], 1);
                    } else {
                        // TODO error handling
                    }
                }
            } else {
                kill(model->ipc->master, SIGMELT);
                break;
            }
        }
    }

//    wait_children();

    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
}

void signal_handler(int signum) {
    sig = signum;
    if (signum == SIGTERM) {
        interrupted = 1;
    }
}

void split(int *atomic_number, int *child_atomic_number) {
#if defined(FISSION_HALF)
    *child_atomic_number = floor((double) *atomic_number / 2);
    *atomic_number = *atomic_number - *child_atomic_number;
#elif defined(FISSION_OTHER)
#else
    *child_atomic_number = rand_between(1, *atomic_number - 1);
    *atomic_number = *atomic_number - *child_atomic_number;
#endif
}