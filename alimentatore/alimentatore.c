#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/ipc.h"
#include "lib/sig.h"
#include "lib/fifo.h"
#include "lib/shmem.h"

void signal_handler(int signum);

struct Model *model;
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


    // Open fifo
    if ((model->res->fifo_fd = fifo_open(FIFO, O_WRONLY)) == -1) {
        exit(EXIT_FAILURE);
    }

    sem_sync(model->ipc->semid, SEM_SYNC);

    char *buf;
    char **argvc;
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    while (!interrupted) {
        nano_sleep(STEP_ALIMENTAZIONE);
        for (int i = 0; !interrupted && i < N_NUOVI_ATOMI; i++) {
            sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
            pid_t child_pid = fork_execve(argvc);
            if (child_pid != -1) {
                write(model->res->fifo_fd, &child_pid, sizeof(pid_t));
            } else {
                // TODO signal master we meltdown :(
                interrupted = 1;
            }
        }
    }

    frargs(argvc, buf);

    wait_children();

    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model->res->fifo_fd != -1) {
        fifo_close(model->res->fifo_fd);
    }
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
}

void signal_handler(int signum) {
    if (signum == SIGTERM) {
        set_sighandler(SIGTERM, SIG_IGN);
        kill(0, SIGTERM);
        interrupted = 1;
    }
}
