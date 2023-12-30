#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#include "master.h"
#include "../libs/sem/sem.h"
#include "../libs/fifo/fifo.h"
#include "../libs/lifo/lifo.h"
#include "../libs/shmem/shmem.h"

sig_atomic_t interrupted = 0;
struct Model *model;

#define INHIBITOR_FLAG  0

static int flags[1] = {
    [INHIBITOR_FLAG] = 0
};

// TODO sigterm handler


int main(int argc, char *argv[]) {
    // immediately register cleanup function
    init();

    // check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp("--inhibitor", argv[i]) == 0) {
            flags[INHIBITOR_FLAG] = 1;
        }
    }

    setbuf(stdout, NULL);   // TODO si vuole?
    setbuf(stderr, NULL);   // TODO si vuole?


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
    if ((fifo_create(FIFO, S_IWUSR | S_IRUSR) == -1) || (model->res->fifo_fd = fifo_open(FIFO, O_RDWR)) == -1)  {
        // fifo_create automatically setups fifo removal at exit,
        // nothing else to be done if fifo_open fails
        exit(EXIT_FAILURE);
    }


    // =========================================
    //             Setup semaphore
    // =========================================
    // let other processes know semaphore set id
    model->ipc->semid = semget(IPC_PRIVATE, SEM_COUNT, S_IWUSR | S_IRUSR);
    if (model->ipc->semid == -1) {
        exit(EXIT_FAILURE);
    }

    int nproc = N_ATOMI_INIT                // atomi
                + flags[INHIBITOR_FLAG]     // inibitore
                + 1                         // alimentatore
                + 0                         // attivatore
                + 1;                        // master

    union semun se;
    se.array = malloc(SEM_COUNT * sizeof(unsigned short));
    se.array[SEM_SYNC] = nproc <= USHRT_MAX ? (short) nproc : 0;
    se.array[SEM_INHIBITOR_ON] = flags[INHIBITOR_FLAG];
    se.array[SEM_INHIBITOR] = 0;
    se.array[SEM_MASTER] = 1;
    se.array[SEM_ATOM] = 1;

    int res = semctl(model->ipc->semid, 0, SETALL, se);
    free(se.array);
    if (res == -1) {
        print(E, "Could not initialize semaphore set.\n");
        exit(EXIT_FAILURE);
    }

    if (nproc > USHRT_MAX) {
        se.val = nproc;
        if (semctl(model->ipc->semid, SEM_SYNC, SETVAL, se) == -1) {
            print(E, "Could not initialize sync semaphore.\n");
            exit(EXIT_FAILURE);
        }
    }


    // =========================================
    //          Forking alimentatore
    // =========================================
    char *buf;
    char **argvc;
    prargs("alimentatore", &argvc, &buf, 1, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    res = fork_execve(argvc);
    frargs(argvc, buf);
    if (res == -1) {
        exit(EXIT_FAILURE);
    }

    // =========================================
    //           Forking attivatore
    // =========================================
    if (flags[INHIBITOR_FLAG]) {
        // TODO spawn attivatore
    }


    // =========================================
    //           Forking inibitore
    // =========================================
    // TODO spawn inibitore


    // =========================================
    //              Forking atoms
    // =========================================
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", model->res->shmid);
    for (int i = 0; res != -1 && i < N_ATOMI_INIT; i++) {
        sprintf(argvc[2], "%d", 123);
        res = fork_execve(argvc);
    }
    frargs(argvc, buf);
    if (res == -1) {
        exit(EXIT_FAILURE);
    }

    // master will fork no more atoms,
    // no need to keep fifo open
    fifo_close(model->res->fifo_fd);
    model->res->fifo_fd = -1;


    // Waiting for child processes
    sem_sync();


    // =========================================
    //              Main logic
    // =========================================
    print(I, "All processes ready, simulation start.\n");


    // =========================================
    //               Cleanup
    // =========================================

    wait_children();

    exit(EXIT_SUCCESS);
}

//void cleanup() {
//    if (model != NULL && model->ipc->semid != -1) {
//        if (semctl(model->ipc->semid, 0, IPC_RMID) == -1) {
//            print(E, "Could not request semaphore set removal.\n");
//        }
//    }
//    if (shmid != -1 && shmaddr != (void *) -1) {
//        shmem_detach(shmaddr);
//    }
//    if (fifo_fd != -1) {
//        fifo_close(fifo_fd);
//    }
//}