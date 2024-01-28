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

static struct SimulationStats sim;
static timer_t timer;

int main(int argc, char *argv[]) {
    // check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp("--inhibitor", argv[i]) == 0) {
            flags[INHIBITOR_FLAG] = 1;
        } else if (strcmp("--no-inh-log", argv[i]) == 0) {
            flags[INHIBITOR_LOG_ON_FLAG] = 0;
        }
    }


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGALRM, SIGMELT);
    sig_handler(SIGTERM, &sigterm_handler);


    // =========================================
    //   Initialize process data and behaviour
    // =========================================
    init(NULL);


    // =========================================
    //           Setup shared memory
    // =========================================
    size_t shmsize = sizeof(struct Config)
                    + sizeof(struct Stats)
                    + sizeof(struct Ipc)
                    + sizeof(struct Lifo);

    if ((model->res->shmid = shmem_create(IPC_PRIVATE, shmsize, S_IWUSR | S_IRUSR | IPC_CREAT)) != -1) {
        model->res->shmaddr = shmem_attach(model->res->shmid);

        // mark shared memory for removal so that
        // as soon as no process is attached
        // to it the OS can recall it
        shmem_rmark(model->res->shmid);
    }

    if (model->res->shmid == -1 || model->res->shmaddr == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    // initialize shared data
    attach_model();
    memset(model->stats, 0, sizeof(struct Stats));
    memset(model->lifo, -1, sizeof(struct Lifo));
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
    // a simple heuristic is to assume the worst case where an atomic number is
    // always divided like so: atom(N) -> parent atom(N - 1) and child atom(1)
    int segment_length = (int) (N_ATOM_MAX - MIN_N_ATOMICO);
    lifo_create(model->lifo, segment_length, sizeof(pid_t), model->ipc->semid, SEM_LIFO);


    // =========================================
    //          Setup initial status
    // =========================================
    memset(&sim.stats, 0, sizeof(struct Stats));
    sim.inhibitor_off = flags[INHIBITOR_FLAG];
    sim.remaining_seconds = SIM_DURATION;
    sim.status = STARTING;


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
        sim.status = MELTDOWN;
        exit(EXIT_SUCCESS);
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
        sim.status = MELTDOWN;
        exit(EXIT_SUCCESS);
    }


    // =========================================
    //           Forking attivatore
    // =========================================
    prargs("attivatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    pid_t child_pid = fork_execv(argvc);
    frargs(argvc, buf);
    if (child_pid == -1) {
        sim.status = MELTDOWN;
        exit(EXIT_SUCCESS);
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
        sim.status = MELTDOWN;
        exit(EXIT_SUCCESS);
    }


    // =========================================
    //       Waiting for child processes
    // =========================================
    print(I, "Waiting for all processes to be ready..\n");
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //            Setup main logic
    // =========================================
    sim.status = RUNNING;

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
    while (sim.status == RUNNING) {
        sigsuspend(&critical);

        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;

        sem_buf(&sops, SEM_MASTER, -1, 0);
        if (sem_op(model->ipc->semid, &sops, 1) == -1) {
            print(E, "Could not acquire master semaphore.\n");
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &sim_curr);
        unsigned long elapsed_millis = 0;
        elapsed_millis += (sim_curr.tv_sec - sim_start.tv_sec) * (long) 1e3;      // converts seconds to milliseconds
        elapsed_millis += (sim_curr.tv_nsec - sim_start.tv_nsec) / (long) 1e6;    // converts nanoseconds to milliseconds
        sim.remaining_seconds = SIM_DURATION - elapsed_millis / (long) 1e3;

        switch (sig) {
            case SIGMELT:
                sim.status = MELTDOWN;
                break;
            case SIGTERM:
                sim.status = TERMINATED;
                break;
            case SIGALRM:
                if (model->stats->curr_energy < ENERGY_DEMAND) {
                    sim.status = BLACKOUT;
                } else {
                    model->stats->curr_energy -= ENERGY_DEMAND;
                    model->stats->used_energy += ENERGY_DEMAND;
                    if (model->stats->curr_energy >= ENERGY_EXPLODE_THRESHOLD) {
                        sim.status = EXPLODE;
                    }
                }

                if (elapsed_millis >= SIM_DURATION * (unsigned long) 1e3) {
                    sim.status = TIMEOUT;
                }
                break;
            default:
                break;
        }

        // copy simulation status for printing
        memcpy(&sim.stats, model->stats, sizeof(struct Stats));
        if ((sim.inhibitor_off = semctl(model->ipc->semid, SEM_INIBITORE_OFF, GETVAL)) == -1) {
            print(E, "Could not check inhibitor status.\n");
        }

        // let simulation continue
        if (sim.status == RUNNING) {
            sem_buf(&sops, SEM_MASTER, +1, 0);
            if (sem_op(model->ipc->semid, &sops, 1) == -1) {
                print(E, "Could not release master semaphore.\n");
            }

            // print simulation status while simulation continues independently
            print_stats(sim);
        }
    }

    exit(EXIT_SUCCESS);
}

void cleanup() {
    // clear misc data
    timer_delete(timer);

    // terminate all child processes
    sig_handler(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    wait_children();

    // print simulation status at the end of the simulation
    memcpy(&sim.stats, model->stats, sizeof(struct Stats));
    if ((sim.inhibitor_off = semctl(model->ipc->semid, SEM_INIBITORE_OFF, GETVAL)) == -1) {
        print(E, "Could not check inhibitor status.\n");
    }
    print_stats(sim);

    DEBUG_BREAKPOINT;

    // detach and remove IPC resources
    if (model != NULL) {
        if (model->lifo->shmid != -1) {
            lifo_delete(model->lifo);
        }
        DEBUG_BREAKPOINT;
        if (model->ipc->semid != -1) {
            sem_delete(model->ipc->semid);
        }
        DEBUG_BREAKPOINT;
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
        DEBUG_BREAKPOINT;
    }

    print(W, "Program ended.\n");
}

static void sigterm_handler() {
    sig = SIGTERM;
}