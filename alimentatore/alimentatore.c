#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/ipc.h"
#include "lib/fifo.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

int running();

int main(int argc, char *argv[]) {
#ifdef D_PID
    print(D, "Alimentatore: %d\n", getpid());
#endif
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();
    sig_handle(NULL, SIGALRM, SIGTERM);


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


    // Open fifo
    if ((model->res->fifo_fd = fifo_open(FIFO, O_WRONLY)) == -1) {
        exit(EXIT_FAILURE);
    }

    sem_sync(model->ipc->semid, SEM_SYNC);

    char *buf;
    char **argvc;
    prargs("atomo", &argvc, &buf, 3, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    sprintf(argvc[3], "%s", "Att");

    struct sembuf sops[2];
    sem_buf(&sops[0], SEM_INIBITORE_ON, 0, IPC_NOWAIT);
    sem_buf(&sops[1], SEM_ALIMENTATORE, -1, 0);

    timer_t timer = timer_start(STEP_ALIMENTAZIONE);
    while (running()) {
        int n_atoms = 0;
        while (n_atoms < N_NUOVI_ATOMI) {
            if (sem_op(model->ipc->semid, sops, 2) == 0 || errno == EAGAIN) {
                mask(SIGALRM);
                sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
                pid_t child_pid = fork();
                switch (child_pid) {
                    case -1:
                        kill(model->ipc->master, SIGMELT);
                        break;
                    case 0:
                        execv(argvc[0], argvc);
                        print(E, "Could not execute %s.\n", argvc[0]);
                        kill(model->ipc->master, SIGMELT);     // TODO set_meaningful_signals if working as expected
                        break;
                    default:
                        n_atoms++;
                        break;
                }
                unmask(SIGALRM);
            } else if (errno == EINTR && sig == SIGALRM) {
                sig = -1;
                break;
            }
            if (sig == SIGALRM) {
                break;
            }
        }
    }
    timer_delete(timer);

    frargs(argvc, buf);

    wait_children();

    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model != NULL) {
        if (model->res->fifo_fd != -1) {
            fifo_close(model->res->fifo_fd);
        }
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}

int running() {
    // while no meaningful signal is received
    while (!sig_is_handled(sig)) {
        // wait for children processes to terminate
        while (wait(NULL) != -1) {
            // if a child atom died, check for its exit status so that
            // other processes can perform their job accordingly
//            struct sembuf sops;
//            sem_buf(&sops, SEM_ALIMENTATORE, +1, 0);
//            if (sem_op(model->ipc->semid, &sops, 1) == -1) {
//                // TODO
//            }
        }

        // if this process has no children
        if (errno == ECHILD) {
            // wait until a signal is received
            // when pause is interrupted by a signal
            pause();
            // break the inner loop so that meaningful
            // signals are checked by the outer one
            break;
        }
    }

    return sig_reset(sig != SIGTERM);
}

