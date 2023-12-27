#include <time.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <printf.h>

#include "util.h"
#include "../console.h"
#include "../ipc/ipc.h"

void nano_sleep(sig_atomic_t *interrupted, long nanos) {
    struct timespec t;
    t.tv_sec = nanos / 1000000000;
    t.tv_nsec = nanos % 1000000000;
    while (!*interrupted && nanosleep(&t, &t) == -1)
        ;
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
        errno_term("Could not execute %s.\n", F_INFO, argv[0]);
    }
    return pid;
}

void errno_term(char *format, int line, char *file, ...) {
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

    raise(SIGTERM);
}

void term(char *format, int line, char *file, ...) {
    va_list args;

    va_start(args, file);
    printf("\033[1;31m");
    fprintf(stderr, E);
    vfprintf(stderr, format, args);
    fprintf(stderr, M "(%s:%d, pid: %5d)\n", file, line, getpid());
    printf("\033[0m");
    va_end(args);

    raise(SIGTERM);
}