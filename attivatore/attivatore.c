#include <time.h>
#include <signal.h>
#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/fifo.h"
#include "lib/util.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

int main(int argc, char *argv[]) {
#ifdef D_PID
    print(D, "Attivatore: %d\n", getpid());
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
    sigset_t critical;
    sig_setup(&mask, &critical, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);


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


    // =========================================
    //              Setup fifo
    // =========================================
    if ((model->res->fifo_fd = fifo_open(FIFO, O_RDONLY)) == -1) {
        exit(EXIT_FAILURE);
    }

    // activate non-blocking fifo read
    int fifo_flags = fcntl(model->res->fifo_fd, F_GETFL, 0);
    fcntl(model->res->fifo_fd, F_SETFL, fifo_flags | O_NONBLOCK);

    // Sem Sync
    sem_sync(model->ipc->semid, SEM_SYNC);


    struct sembuf sops[2];
    timer_t timer = timer_start(STEP_ATTIVATORE);
    while (1) {
        sigsuspend(&critical);

        if (sig == SIGTERM) {
            break;
        }

        sem_buf(&sops[0], SEM_ATTIVATORE, -1, 0);
        if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
            print(E, "Could not acquire SEM_ATTIVATORE semaphore.\n");
            break;
        }

        //

        sem_buf(&sops[0], SEM_MASTER, -1, 0);
        if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
            print(E, "Could not acquire SEM_MASTER semaphore.\n");
        }

        // first try to remove an atom from the lifo (recently activated atoms)
        pid_t atom = -1;
        if (lifo_pop(model->lifo, &atom) == -1) {
            if (fifo_remove(model->res->fifo_fd, &atom, sizeof(pid_t)) == -1) {
                sem_buf(&sops[0], SEM_MASTER, +1, 0);
                sem_buf(&sops[1], SEM_ATTIVATORE, +1, 0);
                if (sem_op(model->ipc->semid, sops, 2) == -1) {
                    print(E, "Could not release SEM_ATTIVATORE semaphore.\n");
                }
            }
        }

        if (atom != -1) {
//            DEBUG_BREAKPOINT;
//            sem_buf(&sops, SEM_MASTER, -1, 0);
//            if (sem_op(model->ipc->semid, &sops, 1) == -1) {
//                print(E, "Could not acquire SEM_MASTER semaphore.\n");
//            }
//            DEBUG_BREAKPOINT;
            if (kill(atom, SIGACTV) == -1) {
                print(E, "Could not activate atom %d.\n", atom);
            } else {
//                model->stats->n_activations++;
            }

            // we do not release SEM_MASTER given
            // the activation transaction has begun

            /*unmask(SIGTERM);
            if (sig == SIGTERM) {
                break;
            }
            mask(SIGTERM);*/
        }
    }
    timer_delete(timer);

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