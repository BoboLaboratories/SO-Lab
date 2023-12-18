#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "ipc.h"
#include "../console.h"
#include "../config/config.h"

extern int errno;
extern struct Model *model;

static struct IpcRes *res;

void init_ipc(struct IpcRes *r, int component) {
    // we set everything to -1 or NULL as each component
    // may keep track of different information
    memset(r, -1, sizeof(struct IpcRes));
    memset(r->addr, 0, 2 * sizeof(void *));
    r->component = component;
    res = r;
}

void attach_model() {
    if ((model = malloc(sizeof(struct Model))) == NULL) {
        errno_fail("Could not allocate model.\n", F_INFO);
    }

    model->config = (struct Config *) res->addr[CTL];
    model->ctl = (struct Control *) model->config + sizeof(struct Config);
    model->atoms = (struct Atoms *) res->addr[MAIN];
}

void attach_shmem(enum Shmem which) {
    res->addr[which] = shmat(res->shmid[which], NULL, 0);
    if (res->addr[which] == (void *) -1) {
        errno_fail("Could not attach shared memory\n", F_INFO);
    }

    #ifdef DEBUG
        printf(D "Attached shared memory (shmid: %d)\n", res->shmid[which]);
    #endif
}

void free_shmem(enum Shmem which) {
    // only master process should request the main shared memory removal
    if (res->component == MASTER && res->shmid[which] != -1) {
        #ifdef DEBUG
            printf(D "Requesting shared memory removal (shmid: %d)\n", res->shmid[which]);
        #endif
        if (shmctl(res->shmid[which], IPC_RMID, NULL) == -1) {
            fprintf(stderr, E "Could not request shared memory removal (shmid: %d)\n", res->shmid[which]);
            ERRNO_PRINT;
        }
    }
}

void detach_shmem(enum Shmem which) {
    if (res->addr[which] != NULL) {
        #ifdef DEBUG
            printf(D "Detaching shared memory (shmid: %d)\n", res->shmid[which]);
        #endif
        if (shmdt(res->addr[which]) == -1) {
            fprintf(stderr, E "Could not detach shared memory (shmid: %d)\n", res->shmid[which]);
            ERRNO_PRINT;
        }
    }
}

void free_ipc() {
    // should some of these free operations fail,
    // this function should not return and the process should not terminate
    // as some resources may still be in use

    detach_shmem(MAIN);
    detach_shmem(CTL);
    free_shmem(MAIN);
    free_shmem(CTL);

    free(model);

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