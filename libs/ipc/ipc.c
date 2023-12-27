#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "ipc.h"
#include "../config.h"
#include "../console.h"

extern int errno;
extern struct Model *model;

static struct IpcRes *res;

void init_ipc(struct IpcRes **r, enum Component component) {
    // we set everything to -1 or NULL as each component
    // may keep track of different information
    res = *r = malloc(sizeof(struct IpcRes));

    memset(res, -1, sizeof(struct IpcRes));
    res->component = component;
    res->addr = NULL;
}

void attach_model() {
    if ((model = malloc(sizeof(struct Model))) == NULL) {
        errno_fail("Could not allocate model.\n", F_INFO);
    }

    model->config = (struct Config *) res->addr;
    model->stats = (struct Stats *) model->config + sizeof(struct Config);

    model->lifo = NULL;
    if (res->component == ATTIVATORE || res->component == ATOMO) {
        model->lifo = (struct Lifo *) model->stats + sizeof(struct Stats);
    }
}

void attach_shmem() {
    res->addr = shmat(res->shmid, NULL, 0);
    if (res->addr == (void *) -1) {
        errno_fail("Could not attach shared memory\n", F_INFO);
    }

    #ifdef DEBUG
        printf(D "Attached shared memory (shmid: %d)\n", res->shmid);
    #endif
}

void free_shmem() {
    // only master process should request the main shared memory removal
    if (res->component == MASTER && res->shmid!= -1) {
        #ifdef DEBUG
            printf(D "Requesting shared memory removal (shmid: %d)\n", res->shmid);
        #endif
        if (shmctl(res->shmid, IPC_RMID, NULL) == -1) {
            fprintf(stderr, E "Could not request shared memory removal (shmid: %d)\n", res->shmid);
            ERRNO_PRINT;
        }
    }
}

void detach_shmem() {
    if (res->addr != NULL) {
        #ifdef DEBUG
            printf(D "Detaching shared memory (shmid: %d)\n", res->shmid);
        #endif
        if (shmdt(res->addr) == -1) {
            fprintf(stderr, E "Could not detach shared memory (shmid: %d)\n", res->shmid);
            ERRNO_PRINT;
        }
    }
}

void open_fifo(int flags) {
    if ((res->fifo_fd = open(FIFO_PATHNAME, flags)) == -1) {
        fprintf(stderr, E "Could not open fifo.\n");
        ERRNO_PRINT;
    }
}

void close_fifo() {
    if (close(res->fifo_fd) == -1) {
        fprintf(stderr, E "Could not close fifo.\n");
        ERRNO_PRINT;
    }
}

void sem_sync(int semid) {
    // TODO what is a signal interrupts? D:
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_flg = 0;

    // signal we are ready
    sops.sem_op = -1;
    semop(semid, &sops, 1);

    // wait for everyone to be ready
    sops.sem_op = 0;
    semop(semid, &sops, 1);
}

void free_ipc() {
    // should some of these free operations fail,
    // this function should not return and the process should not terminate
    // as some resources may still be in use

    detach_shmem();
    free_shmem();

    close_fifo();

    if (res->component == MASTER) {
        if (remove(FIFO_PATHNAME) == -1) {
            fprintf(stderr, E "Could not delete fifo.\n");
            ERRNO_PRINT;
        }
        if (remove(IPC_DIRECTORY) == -1) {
            fprintf(stderr, E "Could not delete ipc directory.\n");
            ERRNO_PRINT;
        }
    }

    free(model);
    free(res);

    #ifdef DEBUG
        printf(D "Freed all IPC resources\n");
    #endif
}

int errno_fail(char *format, int line, char *file, ...) {
    int backup_errno;
    va_list args;

    backup_errno = errno;

    va_start(args, file);
    printf("\033[1;31m");
    fprintf(stderr, E);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, M "errno %d: %s (%s:%d, pid: %5d)\n", backup_errno, strerror(backup_errno), file, line, getpid());
    printf("\033[0m");

    free_ipc();
    exit(EXIT_FAILURE);
}

int fail(char *format, int line, char *file, ...) {
    va_list args;

    va_start(args, file);
    printf("\033[1;31m");
    fprintf(stderr, E);
    vfprintf(stderr, format, args);
    fprintf(stderr, M "(%s:%d, pid: %5d)\n", file, line, getpid());
    printf("\033[0m");
    va_end(args);

    free_ipc();
    exit(EXIT_FAILURE);
}