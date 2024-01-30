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
#include "alimentatore.h"

extern struct Model *model;
extern sig_atomic_t sig;

static char **argvc = NULL;
static char *buf = NULL;
static long n_atoms = 0;
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

        reset();

        while (n_atoms < N_NUOVI_ATOMI) {
            // if inhibitor is present, only fork at most
            // many new atoms as there are in SEM_ALIMENTATORE
            struct sembuf sops[2];
            sem_buf(&sops[0], SEM_INIBITORE_OFF, 0, IPC_NOWAIT);
            sem_buf(&sops[1], SEM_ALIMENTATORE, -1, 0);

            if (sem_op(model->ipc->semid, sops, 2) == 0 || errno == EAGAIN) {
                sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));

                // do not fork unless master is available so that
                // we're sure simulation is in a consistent state
                sem_buf(&sops[0], SEM_MASTER, -1, 0);
                if (sem_op(model->ipc->semid, &sops[0], 1) == -1) {
                    print(E, "Could not acquire master semaphore.\n");
                }

                pid_t atom = fork_execv(argvc);

                // immediately release, regardless of fork result,
                // so that simulation can continue and if fork failed
                // we do not retain control of master semaphore
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

                // if a new step begun, reset the main logic
                sig = -1;
                unmask(SIGALRM);
                if (sig == SIGALRM) {
                    reset();
                }
                mask(SIGALRM);
            } else {
                print(E, "Could not create a new atom.\n");
            }
        }
    }

    exit(EXIT_SUCCESS);
}

static void reset() {
    // resets the number of atoms created in this step
    n_atoms = 0;

    // consume any child process termination status already available
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
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