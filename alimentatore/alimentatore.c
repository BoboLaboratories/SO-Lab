#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "../lib/ipc.h"
#include "../model/model.h"
#include "../lib/sem/sem.h"
#include "../lib/fifo/fifo.h"
#include "../lib/shmem/shmem.h"

void sigterm_handler();

sig_atomic_t interrupted = 0;
struct Model *model;

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


    // Open fifo
    if ((model->res->fifo_fd = fifo_open(FIFO, O_WRONLY)) == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //          Setup signal handler
    // =========================================
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        print(E, "Could not set SIGTERM handler.\n");
    }

    sem_sync(model->ipc->semid, SEM_SYNC);

    char *buf;
    char **argvc;
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    while (!interrupted) {
        nano_sleep(STEP_ALIMENTAZIONE);
        for (int i = 0; !interrupted && i < N_NUOVI_ATOMI; i++) {
            sprintf(argvc[2], "%d", 123);
            if (fork_execve(argvc) == -1) {
                // TODO signal master we meltdown :(
                interrupted = 1;
            }
        }
    }

    frargs(argvc, buf);

    wait_children();

    exit(EXIT_SUCCESS);
}

void sigterm_handler() {
    // TODO signal masking to prevent other signals from interrupting this handler
    // TODO probably not needed as sig_atomic_t is atomic already
    interrupted = 1;
}

void cleanup() {
    if (model->res->fifo_fd != -1) {
        fifo_close(model->res->fifo_fd);
    }
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
}