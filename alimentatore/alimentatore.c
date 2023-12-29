#ifndef ALIMENTATORE
#define ALIMENTATORE
#endif

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "../libs/ipc.h"
#include "../model/model.h"
#include "../libs/sem/sem.h"
#include "../libs/fifo/fifo.h"
#include "../libs/shmem/shmem.h"

void sigterm_handler();

sig_atomic_t interrupted = 0;
struct Model *model;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //          Setup shared memory
    // =========================================
    int shmid;
    if (parse_int(argv[1], &shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    void *shmaddr;
    if ((shmaddr = shmem_attach(shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(shmaddr);


    // Open fifo
    fifo_open(FIFO, O_WRONLY);

    // =========================================
    //          Setup signal handler
    // =========================================
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        print(E, "Could not set SIGTERM handler.\n");
    }

    sem_sync();

    char *buf;
    char **argvc;
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", shmid);
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