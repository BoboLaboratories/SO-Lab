#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "../libs/config.h"
#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"
#include "../libs/model/model.h"
#include "../libs/shmem/shmem.h"

void sigterm_handler();

const enum Component component = ALIMENTATORE;
struct Config *config;
struct Stats *stats;

sig_atomic_t interrupted = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // =========================================
    //          Setup shared memory
    // =========================================
    int shmid;
    if (parse_int(argv[1], &shmid) == -1) {
        print_errno("Could not parse shmid (%s).\n", F_INFO, argv[1]);
        exit(EXIT_FAILURE);
    }

    void *shmaddr;
    if ((shmaddr = shmem_attach(shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    init_model(shmaddr);

    open_fifo(O_WRONLY);


    // =========================================
    //          Setup signal handler
    // =========================================
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        print_errno("Could not set SIGTERM handler.\n", F_INFO);
    }


    int semid;
    if (parse_int(argv[2], &semid) == -1) {
        errno_fail("Could not parse semid.\n", F_INFO);
    }

    sem_sync(semid);


    char *buf;
    char **argvc;
    prargs("atomo", &argvc, &buf, 2, ITC_SIZE);
    sprintf(argvc[1], "%d", res->shmid);

    while (!interrupted) {
        nano_sleep(&interrupted, STEP_ALIMENTAZIONE);
        for (int i = 0; !interrupted && i < N_NUOVI_ATOMI; i++) {
            sprintf(argvc[2], "%d", 123);
            if (fork_execve(argvc) == -1) {
                // TODO signal master we meltdown :(
                interrupted = 1;
            }
        }
    }

    frargs(argvc, buf);

    free_ipc();

    pid_t pid;
    while ((pid = wait(NULL)) != -1)
        ;

    exit(EXIT_SUCCESS);
}

void sigterm_handler() {
    // TODO signal masking to prevent other signals from interrupting this handler
    // TODO probably not needed as sig_atomic_t is atomic already
    interrupted = 1;
}