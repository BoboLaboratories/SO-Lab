#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "model.h"
#include "master.h"
#include "config.h"
#include "lib/sig.h"
#include "lib/sem.h"
#include "lib/fifo.h"
#include "lib/lifo.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

static enum Status status = STARTING;
static timer_t timer;

// TODO if SIGTERM is received, frargs does not happen
int main(int argc, char *argv[]) {
#ifdef D_PID
    print(D, "Master: %d\n", getpid());
#endif
    // check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp("--inhibitor", argv[i]) == 0) {
            flags[INHIBITOR_FLAG] = 1;
        }
    }

    init();


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGALRM, SIGMELT);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sig_handler(SIGTERM, &sigterm_handler);


    // =========================================
    //           Setup shared memory
    // =========================================
    size_t shmize = sizeof(struct Config)
                    + sizeof(struct Stats)
                    + sizeof(struct Ipc)
                    + sizeof(struct Lifo);

    if ((model->res->shmid = shmem_create(IPC_PRIVATE, shmize, S_IWUSR | S_IRUSR | IPC_CREAT)) != -1) {
        model->res->shmaddr = shmem_attach(model->res->shmid);

        // mark shared memory for removal so that
        // as soon as no process is attached
        // to it the OS can recall it
        shmem_rmark(model->res->shmid);
    }

    if (model->res->shmid == -1 || model->res->shmaddr == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(model->res->shmaddr);

    // initialize shared data
    memset(model->stats, 0, sizeof(struct Stats));
    memset(model->ipc, -1, sizeof(struct Ipc));
    model->ipc->master_pid = getpid();
    if (load_config() == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup fifo
    // =========================================
    if ((fifo_create(FIFO, S_IWUSR | S_IRUSR) == -1) /*&& (model->res->fifo_fd = fifo_open(FIFO, O_RDWR)) != -1*/) {
        // TODO questa riga sotto non va proprio amici miei
//        if (fcntl(model->res->fifo_fd, F_SETPIPE_SZ, 70000 * sizeof(pid_t)) == -1) {
//            print(E, "Could not set fifo buffer.\n");
//            exit(EXIT_FAILURE);
//        }
//    } else {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //             Setup semaphores
    // =========================================
    int nproc = N_ATOMI_INIT                // atomi
                + flags[INHIBITOR_FLAG]     // inibitore_pid
                + 1                         // alimentatore_pid
                + 1                         // attivatore
                + 1;                        // master_pid

    int init[SEM_COUNT] = {
            [SEM_INIBITORE_ON] = flags[INHIBITOR_FLAG] ? 0 : 1,
            [SEM_ALIMENTATORE] = 0,
            [SEM_ATTIVATORE] = 1,
            [SEM_INIBITORE] = 0,
            [SEM_SYNC] = nproc,
            [SEM_MASTER] = 1,
            [SEM_ATOM] = 0,
            [SEM_LIFO] = 1
    };

    model->ipc->semid = mksem(IPC_PRIVATE, SEM_COUNT, S_IWUSR | S_IRUSR, init);
    if (model->ipc->semid == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup lifo
    // =========================================
    // TODO 10 come segment_length a 4 occhi chiusi, meglio avere qualche euristica
    mklifo(model->lifo, 10, sizeof(pid_t), model->ipc->semid, SEM_LIFO);


    // =========================================
    //          Forking alimentatore_pid
    // =========================================
    char *buf;
    char **argvc;
    prargs("alimentatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    model->ipc->alimentatore_pid = fork_execv(argvc);
    frargs(argvc, buf);
    if (model->ipc->alimentatore_pid == -1) {
        // TODO meltdown
        exit(EXIT_FAILURE);
    }


    // =========================================
    //           Forking inibitore_pid
    // =========================================
    if (flags[INHIBITOR_FLAG]) {
        prargs("inibitore", &argvc, &buf, 1, ITC_SIZE);
        sprintf(argvc[1], "%d", model->res->shmid);
        model->ipc->inibitore_pid = fork_execv(argvc);
        frargs(argvc, buf);
        if (model->ipc->inibitore_pid == -1) {
            // TODO meltdown
            exit(EXIT_FAILURE);
        }
    }


    // =========================================
    //           Forking attivatore
    // =========================================
    prargs("attivatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    pid_t child_pid = fork_execv(argvc);
    frargs(argvc, buf);
    if (child_pid == -1) {
        // TODO meltdown
        exit(EXIT_FAILURE);
    }


    // =========================================
    //              Forking atoms
    // =========================================
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    for (int i = 0; child_pid != -1 && i < N_ATOMI_INIT; i++) {
        sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
        child_pid = fork_execv(argvc);
    }

    frargs(argvc, buf);
    if (child_pid == -1) {
        // TODO meltdown
        exit(EXIT_FAILURE);
    }


    // =========================================
    //       Waiting for child processes
    // =========================================
    print(I, "Waiting for all processes to be ready..\n");
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //               Main logic
    // =========================================
    print(I, "All processes ready, simulation start.\n");

    status = RUNNING;
    struct sembuf sops;
    timer = timer_start((long) 1e9);
    while (status == RUNNING) {
        sigsuspend(&critical);

        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;

        sem_buf(&sops, SEM_MASTER, -1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        switch (sig) {
            case SIGMELT:
                status = MELTDOWN;
                break;
            case SIGTERM:
                status = TERMINATED;
                break;
            case SIGALRM:
                // TODO handle TIMEOUT
                if (model->stats->curr_energy < ENERGY_DEMAND) {
                    status = BLACKOUT;
                } else {
                    model->stats->curr_energy -= ENERGY_DEMAND;
                    if (model->stats->curr_energy >= ENERGY_EXPLODE_THRESHOLD) {
                        status = EXPLODE;
                    }
                }
                break;
            default:
                break;
        }

        print(I, "S: %d, E: %d, W: %d, A: %d \n", status, model->stats->curr_energy, model->stats->n_wastes,
              model->stats->n_atoms);

        if (status == RUNNING) {
            sem_buf(&sops, SEM_MASTER, +1, 0);
            sem_op(model->ipc->semid, &sops, 1);
        }
    }
    timer_delete(timer);

    sig_handler(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    print(I, "Status: %d\n", status);

    exit(EXIT_SUCCESS);
}

void cleanup() {
    // clear misc data
    timer_delete(timer);

    // detach and remove IPC resources
    if (model != NULL) {
        timer_delete(timer);
        if (model->lifo != NULL) {
            rmlifo(model->lifo);
        }
        if (model->ipc->semid != -1) {
            rmsem(model->ipc->semid);
        }
        if (model->res->fifo_fd != -1) {
            fifo_close(model->res->fifo_fd);
        }
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }

    wait_children();
}

static void sigterm_handler() {
    sig = SIGTERM;
}