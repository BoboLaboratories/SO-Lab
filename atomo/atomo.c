#include <stdlib.h>

#ifdef FISSION_HALF
#include <math.h>
#endif

#include "model.h"
#include "lib/sem.h"
#include "lib/shmem.h"
#include "lib/sig.h"

void signal_handler(int signum);

void split(int *atomic_number, int *child_atomic_number);

sig_atomic_t sig;
struct Model *model;
sig_atomic_t interrupted = 0;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print(E, "Usage: %s <shmid> <atomic-number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int child_atomic_number;
    int atomic_number;

    init();

    if (parse_int(argv[1], &model->res->shmid) == -1) {
        DEBUG_BREAKPOINT;
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

    set_sighandler(SIGACTV, &signal_handler);
    sem_sync(model->ipc->semid, SEM_SYNC);

    pid_t pid = getpid();
    srand(pid);

    char *buf = NULL;
    char **argvc = NULL;
    while (!interrupted) {
        pause();
        if (sig == SIGACTV) {
            if (argvc == NULL) {
                prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
                sprintf(argvc[1], "%d", model->res->shmid);
            }
            split(&atomic_number, &child_atomic_number);
            sprintf(argvc[2], "%d", child_atomic_number);
            pid_t child_pid = fork_execve(argvc);
            if (child_pid != -1) {
                lifo_push(model->lifo, &pid);
                lifo_push(model->lifo, &child_pid);
            } else {
                kill(model->ipc->master, SIGMELT);
                break;
            }
        }
        DEBUG_BREAKPOINT;
    }

    if (argvc != NULL) {
        frargs(argvc, buf);
    }

    wait_children();

    exit(EXIT_SUCCESS);
}

void cleanup() {
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
}

void signal_handler(int signum) {
    sig = signum;
    if (signum == SIGTERM) {
        set_sighandler(SIGTERM, SIG_IGN);
        kill(0, SIGTERM);
        interrupted = 1;
    }
}

void split(int *atomic_number, int *child_atomic_number) {
#if defined(FISSION_HALF)
    *child_atomic_number = floor((double) *atomic_number / 2);
    *atomic_number = *atomic_number - *child_atomic_number;
#elif defined(FISSION_OTHER)
#else
    *child_atomic_number = rand_between(1, *atomic_number - 1);
    *atomic_number = *atomic_number - *child_atomic_number;
#endif
}