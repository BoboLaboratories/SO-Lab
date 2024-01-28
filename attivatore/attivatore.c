#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/fifo.h"
#include "lib/util.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

static timer_t timer;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);


    // =========================================
    //   Initialize process data and behaviour
    // =========================================
    init(argv[1]);


    // =========================================
    //              Setup fifo
    // =========================================
    if ((model->res->fifo_fd = fifo_open(FIFO, O_RDONLY)) == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //         Sync with master_pid process
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);


    struct sembuf sops[2];
    timer = timer_start(STEP_ATTIVATORE);
    while (1) {
        sigsuspend(&critical);

        sem_buf(&sops[0], SEM_ATTIVATORE, -1, 0);
        if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
            print(E, "Could not acquire attivatore semaphore.\n");
            break;
        }

        sem_buf(&sops[0], SEM_MASTER, -1, 0);
        if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
            print(E, "Could not acquire master semaphore.\n");
        }

        // first try to retrieve an atom from the lifo (recently activated atoms)
        pid_t atom = -1;
        if (lifo_pop(model->lifo, &atom) == -1) {
            // otherwise try to retrieve an atom from the fifo (create by alimentatore_pid)
            if (fifo_remove(model->res->fifo_fd, &atom, sizeof(pid_t)) == -1) {
                sem_buf(&sops[0], SEM_MASTER, +1, 0);
                sem_buf(&sops[1], SEM_ATTIVATORE, +1, 0);
                if (sem_op(model->ipc->semid, sops, 2) == -1) {
                    print(E, "Could not retrieve an atom and release semaphores.\n");
                    break;
                }
            }
        }

        if (atom != -1 && kill(atom, SIGACTV) == -1) {
            print(E, "Could not activate atom %d.\n", atom);
        }
    }

    exit(EXIT_FAILURE);
}


void cleanup() {
    // clear misc data
    timer_delete(timer);

    // detach IPC resources
    if (model != NULL) {
        if (model->res->fifo_fd != -1) {
            fifo_close(model->res->fifo_fd);
        }
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}