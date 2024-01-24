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
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

static char **argvc = NULL;
static char *buf = NULL;
static timer_t timer;

int main(int argc, char *argv[]) {
#ifdef D_PID
    print(D, "Alimentatore: %d\n", getpid());
#endif

    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);


    // =========================================
    //            Setup shared memory
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
    //         Sync with master_pid process
    // =========================================
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //                Main logic
    // =========================================
    int terminated = 0;
    struct sembuf sops[2];
    sem_buf(&sops[0], SEM_INIBITORE_ON, 0, IPC_NOWAIT);
    sem_buf(&sops[1], SEM_ALIMENTATORE, -1, 0);

    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);

    timer = timer_start(STEP_ALIMENTAZIONE);
    while (!terminated) {
        sigsuspend(&critical);

        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;

        int n_atoms = 0;
        while (n_atoms < N_NUOVI_ATOMI) {
            if (sem_op(model->ipc->semid, sops, 2) == 0 || errno == EAGAIN) {
                sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
                if (fork_execv(argvc) == -1) {
                    kill(model->ipc->master_pid, SIGMELT);
                    terminated = 1;
                    break;
                }
                n_atoms++;

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