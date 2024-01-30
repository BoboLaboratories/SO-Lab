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
#include "lib/util.h"
#include "lib/shmem.h"
#include "lib/console.h"

extern struct Model *model;
extern sig_atomic_t sig;

static char **argvc = NULL;
static char *buf = NULL;
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
    sigprocmask(SIG_SETMASK, &mask, NULL);


    // =========================================
    //   Initialize process data and behaviour
    // =========================================
    init(argv[1]);


    // =========================================
    //         Sync with master process
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //                Main logic
    // =========================================
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);

    int terminated = 0;
    timer = timer_start(STEP_ALIMENTAZIONE);
    while (!terminated) {
        sigsuspend(&critical);

        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;

        long n_atoms = 0;
        while (n_atoms < N_NUOVI_ATOMI) {
            struct sembuf sops[2];
            sem_buf(&sops[0], SEM_INIBITORE_OFF, 0, IPC_NOWAIT);
            sem_buf(&sops[1], SEM_ALIMENTATORE, -1, 0);
            if (sem_op(model->ipc->semid, sops, 2) == 0 || errno == EAGAIN) {
                sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));

                sem_buf(&sops[0], SEM_MASTER, -1, 0);
                if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
                    print(E, "Could not acquire master semaphore.\n");
                    // TODO release alimentatore
                }

                pid_t atom = fork_execv(argvc);

                sem_buf(&sops[0], SEM_MASTER, +1, 0);
                if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
                    print(E, "Could not release master semaphore.\n");
                }

                if (atom == -1) {
                    kill(model->ipc->master_pid, SIGMELT);
                    terminated = 1;
                    break;
                } else {
                    n_atoms++;
                }

                // is a new step begun, reset the main logic
                // so that we can begin the next step
                sig = -1;
                unmask(SIGALRM);
                if (sig == SIGALRM) {
                    n_atoms = 0;
                    while (waitpid(-1, NULL, WNOHANG) > 0)
                        ;
                }
                mask(SIGALRM);
            }
        }
    }

    exit(EXIT_SUCCESS);
}

void cleanup() {
    // clear misc data
    frargs(argvc, buf);
    timer_delete(timer);

    // detach IPC resources
    if (model != NULL) {
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }

    wait_children();
}