#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "model.h"
#include "config.h"
#include "lib/sig.h"
#include "lib/sem.h"
#include "lib/fifo.h"
#include "lib/lifo.h"
#include "lib/shmem.h"

static void shutdown(int signum, int exit_status);

#define INHIBITOR_FLAG  0

static int flags[1] = {
        [INHIBITOR_FLAG] = 0
};

int MEANINGFUL_SIGNALS[] = { SIGALRM, SIGMELT, -1 };

enum Status status = STARTING;
struct Model *model = NULL;
extern sig_atomic_t sig;

int main(int argc, char *argv[]) {
    print(D, "Master: %d\n", getpid());
    // check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp("--inhibitor", argv[i]) == 0) {
            flags[INHIBITOR_FLAG] = 1;
        }
    }

    init();


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
    model->ipc->master = getpid();
    if (load_config() == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup fifo
    // =========================================
    if ((fifo_create(FIFO, S_IWUSR | S_IRUSR) == -1) || (model->res->fifo_fd = fifo_open(FIFO, O_RDWR)) == -1) {
        // fifo_create automatically setups fifo removal at exit,
        // nothing else to be done if fifo_open fails
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup lifo
    // =========================================
    // TODO 100 come segment_length a 4 occhi chiusi, meglio avere qualche euristica
    mklifo(model->lifo, 10, sizeof(pid_t), model->ipc->semid, SEM_LIFO);


    // =========================================
    //             Setup semaphore
    // =========================================
    // let other processes know semaphore set id
    int nproc = N_ATOMI_INIT                // atomi
                + flags[INHIBITOR_FLAG]     // inibitore
                + 1                         // alimentatore
                + 1                         // attivatore
                + 1;                        // master

    int init[SEM_COUNT] = {
            [SEM_INIBITORE_ON] = flags[INHIBITOR_FLAG] ? 0 : 1,
            [SEM_ALIMENTATORE] = 0,
            [SEM_INIBITORE] = 0,
            [SEM_SYNC] = nproc,
            [SEM_MASTER] = 1,
            [SEM_ATOM] = 1,
            [SEM_LIFO] = 1
    };

    model->ipc->semid = mksem(IPC_PRIVATE, SEM_COUNT, S_IWUSR | S_IRUSR, init);
    if (model->ipc->semid == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //          Forking alimentatore
    // =========================================
    char *buf;
    char **argvc;
    prargs("alimentatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    pid_t child_pid = fork_execve(argvc);
    model->ipc->alimentatore = child_pid;
    frargs(argvc, buf);
    if (child_pid == -1) {
        shutdown(SIGMELT, EXIT_FAILURE);
    }


    // =========================================
    //           Forking inibitore
    // =========================================
    if (flags[INHIBITOR_FLAG]) {
        prargs("inibitore", &argvc, &buf, 1, ITC_SIZE);
        sprintf(argvc[1], "%d", model->res->shmid);
        child_pid = fork_execve(argvc);
        frargs(argvc, buf);
        if (child_pid == -1) {
            shutdown(SIGMELT, EXIT_FAILURE);
        }
    }


    // =========================================
    //           Forking attivatore
    // =========================================
    prargs("attivatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    child_pid = fork_execve(argvc);
    frargs(argvc, buf);
    if (child_pid == -1) {
        shutdown(SIGMELT, EXIT_FAILURE);
    }


    // =========================================
    //              Forking atoms
    // =========================================
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    for (int i = 0; child_pid != -1 && i < N_ATOMI_INIT; i++) {
        sprintf(argvc[2], "%d", rand_between(MIN_N_ATOMICO, N_ATOM_MAX));
        child_pid = fork_execve(argvc);
        if (child_pid != -1) {
            fifo_add(model->res->fifo_fd, &child_pid, sizeof(pid_t));
        }
    }

    frargs(argvc, buf);
    if (child_pid == -1) {
        shutdown(SIGMELT, EXIT_FAILURE);
    }

    // master will fork no more atoms,
    // no need to keep fifo open
    fifo_close(model->res->fifo_fd);
    model->res->fifo_fd = -1;

    // Waiting for child processes
    print(I, "Waiting for all processes to be ready..\n");
    sem_sync(model->ipc->semid, SEM_SYNC);


    // =========================================
    //              Main logic
    // =========================================
    status = RUNNING;
    print(I, "All processes ready, simulation start.\n");

    timer_t timer = timer_start((long) 1e9);
    while (running()) {
        if (sig == SIGMELT) {
            status = MELTDOWN;
        } else {
            struct sembuf sops;
            sem_buf(&sops, SEM_MASTER, -1, 0);
            sem_op(model->ipc->semid, &sops, 1);

            if (model->stats->curr_energy >= ENERGY_DEMAND) {
                model->stats->curr_energy -= ENERGY_DEMAND;
                if (model->stats->curr_energy >= ENERGY_EXPLODE_THRESHOLD) {
                    status = EXPLODE;
                }
            } else {
                status = BLACKOUT;
            }

            // TODO print stats and main logic
            print(I, "E: %d, W: %d, A: %d \n", model->stats->curr_energy, model->stats->n_wastes, model->stats->n_atoms);

            sem_buf(&sops, SEM_MASTER, +1, 0);
            sem_op(model->ipc->semid, &sops, 1);
        }

        // stop simulation by terminated all processes
        if (status != RUNNING) {
            break;
        }
    }
    timer_delete(timer);

    // terminate child processes
    if (status == RUNNING) {
        status = TERMINATED;
    }
    set_sighandler(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    print(I, "Status: %d\n", status);

    // =========================================
    //               Cleanup
    // =========================================
    shutdown(SIGTERM, EXIT_SUCCESS);
}

void cleanup() {
    if (model->ipc->semid != -1) {
        rmsem(model->ipc->semid);
    }
    if (model->res->fifo_fd != -1) {
        fifo_close(model->res->fifo_fd);
    }
    if (model->lifo != NULL) {
        rmlifo(model->lifo);
    }
    if (model->res->shmaddr != (void *) -1) {
        shmem_detach(model->res->shmaddr);
    }
}

static void shutdown(int signum, int exit_status) {
    raise(signum);
    wait_children();
    exit(exit_status);
}

//sig_atomic_t sig = -1;

//int running() {
//    // while no meaningful signal is received
//    // - SIGTERM, means termination
//    // - SIGMELT, means the simulation reached meltdown
//    // - SIGALRM, means a second has passed and stats should be printed
//    while (sig != SIGTERM && sig != SIGALRM && sig != SIGMELT) {
//        // wait for children processes to terminate,
//        // used for clearing N_ATOMI_INIT initial atoms
//        // then simulating a pause by waiting for
//        // other processes that will remain active
//        // for the whole simulation duration
//        while (wait(NULL) != -1)
//            ;
//    }
//
//    int ret = sig != SIGTERM;
//    sig = -1;
//    return ret;
//}

//void signal_handler(int signum) {
//    sig = signum;
//    if (signum == SIGMELT) {
//        status = MELTDOWN;
//    }
//
//    if (signum == SIGMELT || signum == SIGTERM) {
//        set_sighandler(SIGTERM, SIG_IGN);
//        kill(0, SIGTERM);
//    }
//}