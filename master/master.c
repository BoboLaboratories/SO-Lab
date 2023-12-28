#ifndef MASTER
#define MASTER
#endif

#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#include "config.h"
#include "../model/model.h"
#include "../libs/sem/sem.h"
#include "../libs/fifo/fifo.h"
#include "../libs/lifo/lifo.h"
#include "../libs/shmem/shmem.h"

sig_atomic_t interrupted = 0;
struct Model *model;

// TODO sigterm handler

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);   // TODO si vuole?
    setbuf(stderr, NULL);   // TODO si vuole?


    // =========================================
    //          Setup shared memory
    // =========================================
    int shmid;
    void *shmaddr;
    size_t shmize = sizeof(struct Config)
            + sizeof(struct Stats)
            + sizeof(struct Lifo);

    if ((shmid = shmem_create(IPC_PRIVATE, shmize, S_IWUSR | S_IRUSR | IPC_CREAT)) != -1) {
        shmaddr = shmem_attach(shmid);

        // mark shared memory for removal so that
        // as soon as no process is attached
        // to it the OS can recall it
        shmem_rmark(shmid);
    }

    if (shmid == -1 || shmaddr == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    init_model(shmaddr);
    if (load_config() == -1) {
        exit(EXIT_FAILURE);
    }


    // =========================================
    //               Setup fifo
    // =========================================
    fifo_create(FIFO, S_IWUSR | S_IRUSR);
    int fifo_fd = fifo_open(FIFO, O_RDWR);


    // =========================================
    //             Setup semaphores
    // =========================================
    int semid = semget(IPC_PRIVATE, SEM_COUNT, S_IWUSR | S_IRUSR);
    if (semid == -1) {
        // errno_fail("Could not get sync semaphore.\n", F_INFO);
    }

    int nproc = N_ATOMI_INIT /* atomi */
                + 1 /* alimentatore */
                + 0 /* attivatore */
                + 0 /* inibitore */
                + 1 /* master */;

    union semun se;
    se.array = malloc(SEM_COUNT * sizeof(unsigned short));
    se.array[SEM_SYNC] = nproc <= USHRT_MAX ? (short) nproc : 0;
    se.array[SEM_ATOM] = 1;
    se.array[SEM_MASTER] = 1;
    se.array[SEM_INHIBITOR] = 0;
    se.array[SEM_INHIBITOR_ON] = 0; // TODO depends on --inhibitor flag

    int res = semctl(semid, 0, SETALL, se);
    free(se.array);
    if (res == -1) {
        print(E, "Could not initialize semaphore set.\n");
        // TODO exit how
    }

    if (nproc > USHRT_MAX) {
        se.val = nproc;
        if (semctl(semid, SEM_SYNC, SETVAL, se) == -1) {
            print(E, "Could not initialize sync semaphore.\n");
            // TODO exit how
        }
    }


    // =========================================
    //          Forking alimentatore
    // =========================================
    char *buf;
    char **argvc;
    prargs("alimentatore", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", shmid);
    sprintf(argvc[2], "%d", semid);
    if (fork_execve(argvc) == -1) {
        print(E, "Could not fork alimentatore.\n");
    }
    frargs(argvc, buf);


    // =========================================
    //              Forking atoms
    // =========================================
    prargs("atomo", &argvc, &buf, 3, ITC_SIZE);
    sprintf(argvc[1], "%d", shmid);
    sprintf(argvc[2], "%d", semid);
    // argvc[3] will be assigned later for each atom

    for (int i = 0; !interrupted && i < N_ATOMI_INIT; i++) {
        sprintf(argvc[3], "%d", 123);
        // TODO mask SIGTERM?
        if (fork_execve(argvc) == -1) {
            // TODO did we meltdown already? :(
            interrupted = 1;
        }
        // TODO unmask SIGTERM?
    }

    frargs(argvc, buf);

    // master will fork no more atoms,
    // no need to keep fifo open
    fifo_close(fifo_fd);


    // Waiting for child processes
    sem_sync();


    // =========================================
    //              Main logic
    // =========================================



    // =========================================
    //               Cleanup
    // =========================================

    // TODO wait for children before releasing some resources (e.g. semaphores)

    if (shmem_detach(shmaddr) == -1) {
        // TODO should we do anything special about this?
    }

    // TODO la lasciamo qui?
    if (semctl(semid, 0, IPC_RMID) == -1) {
        print(E, "Could not request semaphore set removal.\n");
        // TODO exit or not exit?
    }

    wait_children();

    exit(EXIT_SUCCESS);
}