#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
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


pid_t fork_execve(char **argv) {
    printf("Forking: %s\n", *argv);
    pid_t pid = fork();
    if (pid == 0) {
        execve(argv[0], argv, NULL);
        errno_fail("Could not execute %s.\n", F_INFO, argv[0]);
    }
    return pid;
}

