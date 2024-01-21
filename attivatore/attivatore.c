#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/fifo.h"
#include "lib/util.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

int running();

int main(int argc, char *argv[]) {
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


    // =========================================
    //              Setup fifo
    // =========================================
    if ((model->res->fifo_fd = fifo_open(FIFO, O_RDONLY)) == -1) {
        exit(EXIT_FAILURE);
    }

//    activate non-blocking fifo read
//    int fifo_flags = fcntl(model->res->fifo_fd, F_GETFL, 0);
//    fcntl(model->res->fifo_fd, F_SETFL, fifo_flags | O_NONBLOCK);

    // Sem Sync
    sem_sync(model->ipc->semid, SEM_SYNC);

    struct sembuf sops;
    timer_t timer = timer_start(STEP_ATTIVATORE);
    while (running()) {
        pid_t atom = -1;

        sem_buf(&sops, SEM_ATTIVATORE, -1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            if (errno == EINTR) {
                sem_buf(&sops, SEM_ATTIVATORE, +1, 0);
                sem_op(model->ipc->semid, &sops, 1);
                continue;
            }
        }

        mask(SIGALRM);
        if (lifo_pop(model->lifo, &atom) == -1) {
            unmask(SIGALRM);
            if (sig != SIGALRM) {
                fifo_remove(model->res->fifo_fd, &atom, sizeof(pid_t));
            }
        }
        if (atom != -1) {
            mask(SIGALRM);
            sem_buf(&sops, SEM_MASTER, -1, 0);
            sem_op(model->ipc->semid, &sops, 1);
            if (kill(atom, SIGACTV) == -1) {
                print(E, "Could not activate atom %d.\n", atom);
                sem_buf(&sops, SEM_MASTER, +1, 0);
                sem_op(model->ipc->semid, &sops, 1);
            } else {
                model->stats->n_activations++;
            }
            unmask(SIGALRM);
        } else {
            sem_buf(&sops, SEM_ATTIVATORE, +1, 0);
            sem_op(model->ipc->semid, &sops, 1);
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

int running() {
    while (!sig_is_handled(sig)) {
        pause();
    }

    return sig_reset(sig != SIGTERM);
}