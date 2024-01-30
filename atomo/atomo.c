#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "model.h"
#include "atomo.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/ipc.h"
#include "lib/util.h"
#include "lib/fifo.h"
#include "lib/lifo.h"
#include "lib/shmem.h"
#include "lib/console.h"

extern struct Model *model;
extern sig_atomic_t sig;

static int terminated = 0;
static pid_t ppid;
static pid_t pid;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGACTV, SIGWAST);
    sigprocmask(SIG_SETMASK, &mask, NULL);


    // =========================================
    //   Initialize process data and behaviour
    // =========================================
    init(argv[1]);


    // =========================================
    //            Argument parsing
    // =========================================
    int atomic_number;
    ppid = getppid();
    pid = getpid();

    if (parse_int(argv[2], &atomic_number) == -1) {
        print(E, "Could not parse atomic number (%s).\n", argv[2]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup fifo
    // =========================================
    if ((model->res->fifo_fd = fifo_open(FIFO, O_WRONLY)) == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //         Update simulation stats
    // =========================================
    struct sembuf sops[2];
    sem_buf(&sops[0], SEM_MASTER, -1, 0);
    if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
        print(E, "Could not acquire master_pid semaphore.\n");
        exit(EXIT_FAILURE);
    }

    fifo_add(model->res->fifo_fd, &pid, sizeof(pid_t));
    model->stats->n_atoms++;

    sem_buf(&sops[0], SEM_MASTER, +1, 0);
    if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
        print(E, "Could not release master_pid semaphore.\n");
        exit(EXIT_FAILURE);
    }

    // no longer needed
    fifo_close(model->res->fifo_fd);
    model->res->fifo_fd = -1;


    // =========================================
    //        Sync with other processes
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //                Main logic
    // =========================================
    while (!terminated) {
        sigsuspend(&critical);

        // fission requested
        if (sig == SIGACTV) {
            // acquire master semaphore to proceed with fission
//            sem_buf(&sops[0], SEM_MASTER, -1, 0);
//            if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
//                print(E, "Could not acquire master semaphore.\n");
//            }

            model->stats->n_activations++;

            // if this atom should become waste
            if ((long) atomic_number < MIN_N_ATOMICO) {
                waste(EXIT_NATURAL);
                continue;
            }

            // split atomic numbers
            int child_atomic_number;
            split(&atomic_number, &child_atomic_number);

            // fork child atom
            pid_t child_pid = fork();
            switch (child_pid) {
                case -1: // Meltdown
                    // master semaphore must be release in case of failure
                    sem_buf(&sops[0], SEM_MASTER, +1, 0);
                    if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
                        print(E, "Could not release master semaphore.\n");
                    }
                    // signal master that MELTDOWN happened
                    kill(model->ipc->master_pid, SIGMELT);
                    terminate();
                    break;
                case 0: // Child atom
                    atomic_number = child_atomic_number;
                    ppid = getppid();
                    pid = getpid();
                    break;
                default: { // Parent atom
                    long energy = (atomic_number * child_atomic_number) - max(atomic_number, child_atomic_number);

                    // update simulation stats
                    model->stats->curr_energy += energy;
                    model->stats->n_fissions++;
                    model->stats->n_atoms++;

                    // push both atoms back on lifo
                    if (lifo_push(model->lifo, &pid) == -1) {
                        print(E, "Could not push parent pid to lifo.\n");
                    }
                    if (lifo_push(model->lifo, &child_pid) == -1) {
                        print(E, "Could not push child atom to lifo.\n");
                    }

                    // wake up inhibitor, if activated, to inhibit the energy we just produced
                    sem_buf(&sops[0], SEM_INIBITORE_OFF, 0, IPC_NOWAIT);
                    sem_buf(&sops[1], SEM_INIBITORE, +1, 0);
                    if (sem_op(model->ipc->semid, sops, 2) == -1) {
                        if (errno != EAGAIN) {
                            print(E, "Could not wake up inhibitor.\n");
                        } else {
                            end_activation_cycle();
                        }
                    }
                    break;
                }
            }
        } else if (sig == SIGWAST) {
            // if inhibitor wasted this atom
            waste(EXIT_INHIBITED);
        }
    }

    exit(EXIT_SUCCESS);
}

void cleanup() {
    // detach IPC resources
    if (model != NULL) {
        if (model->res->fifo_fd != -1) {
            fifo_close(model->res->fifo_fd);
        }
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }

    wait_children();
}

static void waste(int status) {
    // we can reach this point either because we became waste
    // because of our atomic number is below MIN_N_ATOMICO
    // or because the inhibitor sent us SIGWAST
    // either case we hold control of SEM_MASTER
    // therefore we're free to update stats
    model->stats->n_wastes++;
    model->stats->n_atoms--;

    end_activation_cycle();

    // only if this atom was wasted because its atomic number was below MIN_N_ATOMICO
    // and this atom is a direct child of master or alimentatore, increment alimentatore
    // semaphore so that inhibitor can keep the number of atoms as stable as possible
    if (status == EXIT_NATURAL && (ppid == model->ipc->master_pid || ppid == model->ipc->alimentatore_pid)) {
        struct sembuf sops[2];
        sem_buf(&sops[0], SEM_INIBITORE_OFF, 0, IPC_NOWAIT);
        sem_buf(&sops[1], SEM_ALIMENTATORE, +1, 0);
        if (sem_op(model->ipc->semid, sops, 2) == -1) {
            if (errno != EAGAIN) {
                print(E, "Could not give work back to alimentatore.\n");
            }
        }
    }

    terminate();
}

static void end_activation_cycle() {
    struct sembuf sops;

    // first, give back control to master process so that it
    // takes precedence to perform any pending control operation
    sem_buf(&sops, SEM_MASTER, +1, 0);
    if (sem_op(model->ipc->semid, &sops, 1) == -1) {
        print(E, "Could not release master semaphore.\n");
    }

    // only then, give work back to attivatore
    sem_buf(&sops, SEM_ATTIVATORE, +1, 0);
    if (sem_op(model->ipc->semid, &sops, 1) == -1) {
        print(E, "Could not increase attivatore semaphore.\n");
    }
}

static void split(int *atomic_number, int *child_atomic_number) {
    srand(pid);
    *child_atomic_number = rand_between(1, *atomic_number - 1);
    *atomic_number = *atomic_number - *child_atomic_number;
}

static void terminate() {
    terminated = 1;

    // we could receive SIGTERM from master while already performing exit
    // operations (e.g. in case we were the atom that caused MELTDOWN)
    // or if we received SIGWAST and the next atom causes MELTDOWN
    mask(SIGTERM);
}