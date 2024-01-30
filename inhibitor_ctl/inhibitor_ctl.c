#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "lib/sem.h"
#include "lib/ipc.h"
#include "lib/console.h"

static int start();
static int stop();

static int semid;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(E, "Usage: %s <start|stop|toggle>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // get the simulation semaphore set key
    key_t key;
    if ((key = ftok(FIFO, FTOK_PROJ)) == -1) {
        print(W, "No running simulation found.\n");
        exit(EXIT_FAILURE);
    };

    // get the simulation semaphore set
    if ((semid = semget(key, SEM_COUNT, S_IWUSR | S_IRUSR)) == -1) {
        print(E, "Could not retrieve semaphore.\n");
        exit(EXIT_FAILURE);
    }

    // briefly pause simulation while we change inhibitor status
    struct sembuf sops;
    sem_buf(&sops, SEM_MASTER, -1, 0);
    if (sem_op(semid, &sops, 1) == -1) {
        print(E, "Could not acquire master semaphore.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp("toggle", argv[1]) == 0) {
        if (stop() == -1) {
            if (start() == -1) {
                print(E, "Could not toggle inhibitor.\n");
            }
            print(I, "Inhibitor started.\n");
        } else {
            print(I, "Inhibitor stopped.\n");
        }
    } else if (strcmp("start", argv[1]) == 0) {
        if (start() == -1) {
            print(W, "Inhibitor was already started.\n");
        } else {
            print(I, "Inhibitor started.\n");
        }
    } else if (strcmp("stop", argv[1]) == 0) {
        if (stop() == -1) {
            print(W, "Inhibitor was already stopped.\n");
        } else {
            print(I, "Inhibitor stopped.\n");
        }
    }

    // resume simulation
    sem_buf(&sops, SEM_MASTER, +1, 0);
    if (sem_op(semid, &sops, 1) == -1) {
        print(E, "Could not release master semaphore.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

static int stop() {
    struct sembuf sops[2];
    sem_buf(&sops[0], SEM_INIBITORE_OFF, 0, IPC_NOWAIT);        // if   OFF != 0
    sem_buf(&sops[1], SEM_INIBITORE_OFF, +1, 0);                // then OFF == 1
    if (sem_op(semid, sops, 2) == -1) {
        if (errno != EAGAIN) {                                  // if sops[0] gives EAGAIN, inhibitor is already OFF
            print(E, "Could not stop inhibitor.\n");
        }
        return -1;
    }
    return 0;
}

static int start() {
    struct sembuf sops;
    sem_buf(&sops, SEM_INIBITORE_OFF, -1, IPC_NOWAIT);          // try to set inhibitor to ON, if fails it was already ON
    if (sem_op(semid, &sops, 1) == -1) {
        if (errno != EAGAIN) {
            print(E, "Could not start inhibitor\n");
        }
        return -1;
    }

    // now that inhibitor is on, alimentatore should keep
    // the number of atoms as low as possibile and the
    // only "assumption" we can make is that we have
    // room for a single fork operation
    union semun se;
    se.val = 1;
    semctl(semid, SEM_ALIMENTATORE, SETVAL, se);

    return 0;
}