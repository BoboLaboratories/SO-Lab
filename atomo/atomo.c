#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/shmem.h"
#include "lib/sig.h"
#include "lib/fifo.h"

int running();

void waste(int status);

void split(int *atomic_number, int *child_atomic_number);

extern struct Model *model;
extern sig_atomic_t sig;

pid_t ppid;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int atomic_number;

    init();
    sig_handle(NULL, SIGACTV, SIGWAST, SIGTERM);

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


    if ((model->res->fifo_fd = fifo_open(FIFO, O_WRONLY)) == -1) {
        exit(EXIT_FAILURE);
    }
    // =========================================
    //              Update stats
    // =========================================
    struct sembuf sops[2];
    sem_buf(&sops[0], SEM_MASTER, -1, 0);
    sem_op(model->ipc->semid, &sops[0], 1);

    model->stats->n_atoms++;
    pid_t tmp = getpid();
    if (fifo_add(model->res->fifo_fd, &tmp, sizeof(pid_t)) == -1) {
        print(E, "Could not insert atom %d in FIFO.\n", tmp);
    }

    sem_buf(&sops[0], SEM_MASTER, +1, 0);
    sem_op(model->ipc->semid, &sops[0], 1);

    // =========================================
    //        Sync with other processes
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);

    ppid = getppid();
    // =========================================
    //                Main logic
    // =========================================
    while (running()) {
        mask(SIGACTV, SIGWAST);
        // if inhibitor wasted this atom
        if (sig == SIGWAST) {
            waste(ATOM_EXIT_INHIBITED);
        }

        // if fission was requested
        if (sig == SIGACTV) {
            // if this atom should become waste
            if (atomic_number < MIN_N_ATOMICO) {
//                print(W, "Attivatore +1\n");
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
                    break;
                default: {
                    // Parent atom
                    long energy = (atomic_number * child_atomic_number) - max(atomic_number, child_atomic_number);
                    model->stats->curr_energy += energy;
                    model->stats->n_fissions++;
                    model->stats->n_atoms++;

                    pid_t pid = getpid();
                    lifo_push(model->lifo, &pid);
                    lifo_push(model->lifo, &child_pid);

                    // wake up inhibitor, if activated, to inhibit the energy we just produced
                    sem_buf(&sops[0], SEM_INIBITORE_ON, 0, IPC_NOWAIT);
                    sem_buf(&sops[1], SEM_INIBITORE, +1, 0);
                    if (sem_op(model->ipc->semid, sops, 2) == -1) {
                        if (errno == EAGAIN) {
                            // if inhibitor is deactivated, give control back to master process
                            sem_buf(&sops[0], SEM_MASTER, +1, 0);
                            sem_op(model->ipc->semid, &sops[0], 1);

                            sem_buf(&sops[0], SEM_ATTIVATORE, +1, 0);
                            sem_op(model->ipc->semid, &sops[0], 1);
                        } else {
                            // TODO
                        }
                    }
                    break;
                }
            }
        }
        sig = -1;
        unmask(SIGACTV, SIGWAST);
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

int running() {
    // while no meaningful signal is received
    // - SIGTERM, means termination
    // - SIGACTV, means fission was requested
    // - SIGWAST, means this atom was wasted
    while (!sig_is_handled(sig)) {
        // wait for children processes to terminate
        while (wait(NULL) != -1)
            ;

        if (errno == ECHILD) {
            // wait until a signal is received
            pause();
            // when pause is interrupted by a signal,
            // break the inner loop so that meaningful
            // signals are checked by the outer one
            break;
        }
    }

    return sig != SIGTERM;
}

void waste(int status) {
    /*if (ppid == model->ipc->master || ppid == model->ipc->alimentatore) {
        struct sembuf sops;
        sem_buf(&sops, SEM_ALIMENTATORE, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);
    }*/

    model->stats->n_wastes++;
    model->stats->n_atoms--;

    if (status == ATOM_EXIT_NATURAL) {
        struct sembuf sops;
        sem_buf(&sops, SEM_MASTER, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        sem_buf(&sops, SEM_ATTIVATORE, +1, 0);
        sem_op(model->ipc->semid, &sops, 1);
    }

    exit(status);
}

void split(int *atomic_number, int *child_atomic_number) {
    srand(getpid());
    *child_atomic_number = rand_between(1, *atomic_number - 1);
    *atomic_number = *atomic_number - *child_atomic_number;
}