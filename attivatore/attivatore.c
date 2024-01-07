#include <signal.h>
#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/fifo.h"
#include "lib/util.h"
#include "lib/shmem.h"

void signal_handler(int signum);

struct Model *model = NULL;
sig_atomic_t interrupted = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();


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


    while (!interrupted) {
        nano_sleep(STEP_ATTIVATORE);
        pid_t atom = -1;
        if (lifo_pop(model->lifo, &atom) == -1) {
            if (fifo_remove(model->res->fifo_fd, &atom, sizeof(pid_t)) == -1) {
                // TODO: Error handle (check errno == EAGAIN)
                // print(W, "Lifo and Fifo are empty, skipping stuff\n");
                continue;
            }
        }
        if (atom != -1) {
            kill(atom, SIGACTV);
        }
    }

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

void signal_handler(int signum) {
    if (signum == SIGTERM) {
        interrupted = 1;
    }
}