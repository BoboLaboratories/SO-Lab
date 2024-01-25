#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "stats.h"
#include "model.h"
#include "master.h"
#include "config.h"
#include "lib/sig.h"
#include "lib/sem.h"
#include "lib/util.h"
#include "lib/fifo.h"
#include "lib/lifo.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

static enum Status status = STARTING;
static timer_t timer;

// TODO if SIGTERM is received, frargs does not happen
int main(int argc, char *argv[]) {
    // check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp("--inhibitor", argv[i]) == 0) {
            flags[INHIBITOR_FLAG] = 1;
        } else if (strcmp("--no-inh-log", argv[i]) == 0) {
            flags[INHIBITOR_LOG_ON_FLAG] = 0;
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
    if (fifo_create(FIFO, S_IWUSR | S_IRUSR) == -1) {
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
            [SEM_INIBITORE_OFF] = flags[INHIBITOR_FLAG] ? 0 : 1,
            [SEM_ALIMENTATORE] = 0,
            [SEM_ATTIVATORE] = 1,
            [SEM_INIBITORE] = 0,
            [SEM_SYNC] = nproc,
            [SEM_MASTER] = 1,
            [SEM_ATOM] = 0,
            [SEM_LIFO] = 1
    };

    key_t key;
    if ((key = ftok(FIFO, FTOK_PROJ)) == -1) {
        print(E, "Could not generate ftok key.\n");
        exit(EXIT_FAILURE);
    };

    model->ipc->semid = sem_create(key, SEM_COUNT, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR, init);
    if (model->ipc->semid == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup lifo
    // =========================================
    // TODO 10 come segment_length a 4 occhi chiusi, meglio avere qualche euristica
    lifo_create(model->lifo, 10, sizeof(pid_t), model->ipc->semid, SEM_LIFO);


    // =========================================
    //          Forking alimentatore
    // =========================================
    char *buf;
    char **argvc;
    prargs("alimentatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    model->ipc->alimentatore_pid = fork_execv(argvc);
    frargs(argvc, buf);
    if (model->ipc->alimentatore_pid == -1) {
        status = MELTDOWN;
        exit(EXIT_FAILURE);
    }


    // =========================================
    //           Forking inibitore
    // =========================================
    prargs("inibitore", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    sprintf(argvc[2], "%d", flags[INHIBITOR_LOG_ON_FLAG]);
    model->ipc->inibitore_pid = fork_execv(argvc);
    frargs(argvc, buf);
    if (model->ipc->inibitore_pid == -1) {
        status = MELTDOWN;
        exit(EXIT_FAILURE);
    }


    // =========================================
    //           Forking attivatore
    // =========================================
    prargs("attivatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    pid_t child_pid = fork_execv(argvc);
    frargs(argvc, buf);
    if (child_pid == -1) {
        status = MELTDOWN;
        exit(EXIT_FAILURE);
    }


    // =========================================
    //              Forking atoms
    // =========================================
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    for (unsigned long i = 0; child_pid != -1 && i < N_ATOMI_INIT; i++) {
        sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
        child_pid = fork_execv(argvc);
    }

    frargs(argvc, buf);
    if (child_pid == -1) {
        status = MELTDOWN;
        exit(EXIT_FAILURE);
    }


    // =========================================
    //       Waiting for child processes
    //          and setup main logic
    // =========================================
    print(I, "Waiting for all processes to be ready..\n");
    sem_sync(model->ipc->semid, SEM_SYNC);

    status = RUNNING;
    struct sembuf sops;

    struct timespec sim_start;
    struct timespec sim_curr;
    memset(&sim_start, 0, sizeof(struct timespec));
    memset(&sim_curr, 0, sizeof(struct timespec));

    clock_gettime(CLOCK_MONOTONIC_RAW, &sim_start);

    // =========================================
    //               Main logic
    // =========================================
    print(I, "All processes ready, simulation start.\n");

    timer = timer_start((long) 1e9);
    while (status == RUNNING) {
        sigsuspend(&critical);

        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;

        sem_buf(&sops, SEM_MASTER, -1, 0);
        sem_op(model->ipc->semid, &sops, 1);

        clock_gettime(CLOCK_MONOTONIC_RAW, &sim_curr);
        unsigned long elapsed_millis = 0;
        elapsed_millis += (sim_curr.tv_sec - sim_start.tv_sec) * (long) 1e3;      // converts seconds to milliseconds
        elapsed_millis += (sim_curr.tv_nsec - sim_start.tv_nsec) / (long) 1e6;    // converts nanoseconds to milliseconds

        struct PrintableStats prnt = {
                .remaining_seconds = SIM_DURATION - elapsed_millis / (long) 1e3,
        };

        switch (sig) {
            case SIGMELT:
                status = MELTDOWN;
                break;
            case SIGTERM:
                status = TERMINATED;
                break;
            case SIGALRM:
                if (model->stats->curr_energy < ENERGY_DEMAND) {
                    status = BLACKOUT;
                } else {
                    model->stats->curr_energy -= ENERGY_DEMAND;
                    model->stats->used_energy += ENERGY_DEMAND;
                    if (model->stats->curr_energy >= ENERGY_EXPLODE_THRESHOLD) {
                        status = EXPLODE;
                    }
                }

                if (elapsed_millis >= SIM_DURATION * (unsigned long) 1e3) {
                    status = TIMEOUT;
                }
                break;
            default:
                break;
        }

        // copy prnt for printing
        prnt.status = status;
        memcpy(&prnt.stats, model->stats, sizeof(struct Stats));
        if ((prnt.inhibitor_off = semctl(model->ipc->semid, SEM_INIBITORE_OFF, GETVAL)) == -1) {
            print(E, "Could not check inhibitor status.\n");
        }

        // let simulation continue
        if (status == RUNNING) {
            sem_buf(&sops, SEM_MASTER, +1, 0);
            sem_op(model->ipc->semid, &sops, 1);
        }

        // print prnt while simulation goes ahead
        print_stats(prnt);
    }
    timer_delete(timer);

    sig_handler(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    exit(EXIT_SUCCESS);
}

void cleanup() {
    // clear misc data
    timer_delete(timer);
    // detach and remove IPC resources
    if (model != NULL) {
        timer_delete(timer);
        if (model->lifo != NULL) {
            lifo_delete(model->lifo);
        }
        if (model->ipc->semid != -1) {
            sem_delete(model->ipc->semid);
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