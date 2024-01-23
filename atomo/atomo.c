#include <errno.h>
#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/shmem.h"
#include "lib/sig.h"
#include "lib/fifo.h"

void waste(int status);
void split(int *atomic_number, int *child_atomic_number);

extern struct Model *model;
extern sigset_t signals;
extern sig_atomic_t sig;

static pid_t ppid;
static pid_t pid;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGACTV, SIGWAST, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, NULL);


    // =========================================
    //            Argument parsing
    // =========================================
    int atomic_number;
    ppid = getppid();
    pid = getpid();

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


    // =========================================
    //               Setup FIFO
    // =========================================
    if ((model->res->fifo_fd = fifo_open(FIFO, O_WRONLY)) == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //              Update stats
    // =========================================
    struct sembuf sops[2];
    sem_buf(&sops[0], SEM_MASTER, -1, 0);
    if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
        print(E, "Could not acquire master semaphore.\n");
        exit(EXIT_FAILURE);
    }

    model->stats->n_atoms++;
    fifo_add(model->res->fifo_fd, &pid, sizeof(pid_t));

    sem_buf(&sops[0], SEM_MASTER, +1, 0);
    if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
        print(E, "Could not release master semaphore.\n");
        exit(EXIT_FAILURE);
    }


    // =========================================
    //        Sync with other processes
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //                Main logic
    // =========================================

    while (1) {
        sigsuspend(&critical);

        if (sig == SIGTERM) {
            break;
        } else if (sig == SIGWAST) {
            waste(ATOM_EXIT_INHIBITED);
        } else if (sig == SIGACTV) {
            // if fission was requested
            // if this atom should become waste
            if (atomic_number < MIN_N_ATOMICO) {
                waste(ATOM_EXIT_NATURAL);
            }

            int child_atomic_number;
            split(&atomic_number, &child_atomic_number);

            pid_t child_pid = fork();
            switch (child_pid) {
                case -1:
                    // Meltdown
                    kill(model->ipc->master, SIGMELT);
                    break;
                case 0:
                    // Child atom
                    atomic_number = child_atomic_number;
                    ppid = getppid();
                    pid = getpid();
                    break;
                default: {
                    // Parent atom
                    long energy = (atomic_number * child_atomic_number) - max(atomic_number, child_atomic_number);

                    // update stats
                    model->stats->curr_energy += energy;
                    model->stats->n_fissions++;
                    model->stats->n_atoms++;

                    // push both atoms back on lifo
                    if (lifo_push(model->lifo, &pid) == -1) {
                        print(E, "Could not push parent pid to lifo.\n");
                        // TODO
                    }
                    if (lifo_push(model->lifo, &child_pid) == -1) {
                        print(E, "Could not push child atom to lifo.\n");
                        // TODO
                    }

                    // wake up inhibitor, if activated, to inhibit the energy we just produced
                    sem_buf(&sops[0], SEM_INIBITORE_ON, 0, IPC_NOWAIT);
                    sem_buf(&sops[1], SEM_INIBITORE, +1, 0);
                    if (sem_op(model->ipc->semid, sops, 2) == -1) {
                        if (errno == EAGAIN) {
                            sem_end_activation(model->ipc->semid);
                        }
                    }
                    break;
                }
            }
        }
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

void waste(int status) {
    model->stats->n_wastes++;
    model->stats->n_atoms--;

    if(model->stats->n_atoms < 0) {
        print(D, "WTF is happening?\n");
    }

    struct sembuf sops;
    if (status == ATOM_EXIT_INHIBITED) {
        sem_buf(&sops, SEM_ATOM, -1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print(E, "Could not update stats.\n");
        }
    } else if (status == ATOM_EXIT_NATURAL) {
        sem_end_activation(model->ipc->semid);
    }

    if (ppid == model->ipc->master || ppid == model->ipc->alimentatore) {
        sem_buf(&sops, SEM_ALIMENTATORE, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);
    }

    exit(EXIT_SUCCESS);
}

void split(int *atomic_number, int *child_atomic_number) {
    srand(getpid());
    *child_atomic_number = rand_between(1, *atomic_number - 1);
    *atomic_number = *atomic_number - *child_atomic_number;
}