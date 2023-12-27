#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#include "config.h"
#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"
#include "../libs/shmem/shmem.h"
#include "../libs/model/model.h"

void setup_ipc();
void setup_shmem();
void setup_fifo();
void fail();

const enum Component component = MASTER;
struct Config *config;
struct Stats *stats;

sig_atomic_t interrupted = 0;



int main(int argc, char *argv[]) {
    setbuf(stdout, NULL); // TODO si vuole?
    setbuf(stderr, NULL); // TODO si vuole?

    // =========================================
    //          Setup IPC directory
    // =========================================
    if ((res.ipc_dir = mkdir(IPC_DIRECTORY, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH)) == -1) {
        errno_term("Could not create IPC directory.\n", F_INFO);
        exit(EXIT_FAILURE);
    }


    // =========================================
    //          Setup shared memory
    // =========================================
    size_t shmize = sizeof(struct Config)
            + sizeof(struct Stats)
            + sizeof(struct Lifo);
    if ((res.shmid = shmem_create(IPC_PRIVATE, shmize, S_IWUSR | S_IRUSR | IPC_CREAT)) == -1) {
        fail();
    }

    if ((res.shmaddr = shmem_attach(res.shmid)) == (void *) -1) {
        // rm .ipc

        shmem_remove(shmid);
    }



    init_model(shmaddr);

    // END


    setup_ipc();
    attach_model();
    load_config();

    open_fifo(O_RDWR);


    // =========================================
    //          Setup sync semaphore
    // =========================================

    int nproc = N_ATOMI_INIT /* atomi */
                + 1 /* alimentatore */
                + 0 /* attivatore */
                + 0 /* inibitore */
                + 1 /* master */;

    printf(D "nproc: %d\n", nproc);

    int semid = semget(IPC_PRIVATE, 1, S_IWUSR | S_IRUSR);
    if (semid == -1) {
        errno_fail("Could not get sync semaphore.\n", F_INFO);
    }

    union semun se;
    se.val = nproc;
    semctl(semid, 0, SETVAL, se);


    // =========================================
    //          Forking alimentatore
    // =========================================

    char *buf;
    char **argvc;
    prargs("alimentatore", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", res->shmid);
    sprintf(argvc[2], "%d", semid);
    if (fork_execve(argvc) == -1) {
        printf("Could not fork alimentatore.\n");
    }
    frargs(argvc, buf);


    // =========================================
    //              Forking atoms
    // =========================================

    prargs("atomo", &argvc, &buf, 3, ITC_SIZE);
    sprintf(argvc[1], "%d", res->shmid);
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


    // Waiting for child processes
    sem_sync(semid);


    // =========================================
    //             Do other stuff
    // =========================================

    pid_t pid;
    ssize_t result;
    while ((result = read(res->fifo_fd, &pid, sizeof(pid_t))) != -1 && pid != -1) {
        printf("Pid: %d\n", pid);
    }
    if (result == -1) {
        errno_fail("Failed to read.", F_INFO);
    }

    free_ipc();
}

void setup_ipc() {


    init_ipc(&res, MASTER);
    setup_shmem();

    setup_fifo();
}

void setup_shmem() {
    size_t size = sizeof(struct Config) + sizeof(struct Stats);
    res->shmid = shmget(IPC_PRIVATE, size, 0666 | IPC_CREAT);
    if (res->shmid == -1) {
        errno_fail("Could not create shared memory.\n", F_INFO);
    }

    #ifdef DEBUG
        printf(D "Obtained shared memory from OS (shmid: %d)\n", res->shmid);
    #endif

    attach_shmem();
}

void setup_fifo() {
    if (mkfifo(FIFO_PATHNAME, S_IWUSR | S_IRUSR) == -1) {
        errno_fail("Could not create fifo.\n", F_INFO);
    }
}