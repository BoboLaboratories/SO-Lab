#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/ipc.h"
#include "lib/fifo.h"
#include "lib/shmem.h"

int running();

void signal_handler(int signum);

struct Model *model = NULL;
sig_atomic_t sig = -1;

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

    timer_t timer = timer_start(STEP_ALIMENTAZIONE);
    while (running()) {
        for (int i = 0; sig != SIGTERM && i < N_NUOVI_ATOMI; i++) {
            sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
            pid_t child_pid = fork_execve(argvc);
            if (child_pid != -1) {
                fifo_add(model->res->fifo_fd, &child_pid, sizeof(pid_t));
            } else {
                kill(model->ipc->master, SIGMELT);
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
    // - SIGTERM, means termination
    // - SIGALRM, means STEP_ALIMENTAZIONE expired
    while (sig != SIGTERM && sig != SIGALRM) {
        // wait for children processes to terminate
        while (wait(NULL) == -1) {
            // if this process has no children
            if (errno == ECHILD) {
                // wait until a signal is received
                pause();
                // when pause is interrupted by a signal,
                // break the inner loop so that meaningful
                // signals are checked by the outer one
                break;
            }
        }
    }

    int ret = sig != SIGTERM;
    sig = -1;
    return ret;
}

void signal_handler(int signum) {
    sig = signum;
}
