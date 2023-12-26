#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "mira.h"

static void *addr;

int *attach_shmem(int shmid) {
    addr = shmat(shmid, NULL, 0);
    if (addr == (void *) -1) {
        printf("Could not attach shared memory.\n");
        exit(EXIT_FAILURE);
    }
    return (int *) addr;
}

void detach_shmem() {
    if (shmdt(addr) == -1) {
        printf("Could not detach shared memory.\n");
        exit(EXIT_FAILURE);
    }
}

void prargs(char *executable, char ***argv, char **buf, int vargs, size_t elemsize) {
    *argv = calloc(vargs + 2, sizeof(char *));
    *buf = calloc(vargs, elemsize);

    (*argv)[0] = executable;
    for (int i = 0; i < vargs; i++) {
        // operator precedence: *buf -> (*buf)[...] -> &(*buf)[...]
        (*argv)[i + 1] = &(*buf)[i * elemsize];
    }
    (*argv)[vargs + 1] = NULL;
}

void frargs(char **argv, char *buf) {
    free(argv);
    free(buf);
}

pid_t fork_execve(char **argv) {
    pid_t pid = fork();
    if (pid == 0) {
        execve(argv[0], argv, NULL);
        printf("Could not execute %s.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    return pid;
}

int parse_long(char *raw, long *dest) {
    char *endptr;
    errno = 0;
    *dest = strtol(raw, &endptr, 10);
    int error = (
            errno == ERANGE     // overflow
            || raw == endptr    // no conversion (no characters read)
            || *endptr          // extra characters at the end
    );
    return error ? -1 : 0;
}

int parse_int(char *raw, int *dest) {
    long tmp;
    if (parse_long(raw, &tmp) != -1) {
        if (tmp <= INT_MAX) {
            *dest = (int) tmp;
            return 0;
        }
        errno = ERANGE;
    }
    return -1;
}