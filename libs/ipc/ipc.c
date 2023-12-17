#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "ipc.h"
#include "../console.h"

extern int errno;
static struct IpcRes *res;

void init_ipc(struct IpcRes *r, int component) {
    // we set everything to -1 or NULL as each component
    // may keep track of different information
    memset(r, -1, sizeof(struct IpcRes));
    memset(r->adrr, 0, 2 * sizeof(void *));
    r->component = component;
    res = r;
}

void attach_shmem(int id) {
    res->adrr[id] = shmat(res->shmid[id], NULL, 0);
    if (res->adrr[id] != (void *) -1) {
        // errno_fail("Could not attach shared memory.\n", F_INFO);
    }
}

void free_ipc() {
    // should some of these free operations fail,
    // this function should not return and the process should not terminate
    // as some resources may still be in use

    // every process should detach from the main shared memory
    if (res->adrr[1] != NULL) {
        if (shmdt(res->adrr[1]) == -1) {
            fprintf(stderr, E "Could not detach main shared memory.\n");
            ERRNO_PRINT;
        }
    }

    // only master process should request the main shared memory removal
    if (res->component == MASTER && res->shmid[1] != -1) {
        if (shmctl(res->shmid[1], IPC_RMID, NULL) == -1) {
            fprintf(stderr, E "Could not request main shared memory deletion.\n");
            ERRNO_PRINT;
        }
    }
}

int errno_fail(char *format, int line, char *file, ...) {
    int backup_errno;
    va_list args;

    backup_errno = errno;

    va_start(args, file);
    fprintf(stderr, E);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    fprintf(stderr, M "Errno %d: %s (%s:%d, pid:%5d)\n", backup_errno, strerror(backup_errno), file, line, getpid());

    free_ipc();
    exit(EXIT_FAILURE);
}

int fail(char *format, int line, char *file, ...) {
    va_list args;

    va_start(args, file);
    fprintf(stderr, E);
    vfprintf(stderr, format, args);
    fprintf(stderr, " (%s:%d)\n" M "Exit failure\n", file, line);
    va_end(args);

    free_ipc();
    exit(EXIT_FAILURE);
}