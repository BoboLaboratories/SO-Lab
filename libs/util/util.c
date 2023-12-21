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

void nano_sleep(long nanos, sig_atomic_t *interrupted) {
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

char **prargs(char *buf, char *executable, char *format, ...) {
    size_t argc = 2;    // number of total arguments
    va_list args;
    char **argv;
    int i = 0;

    // increase argc by the required amount of varargs
    argc += format ? parse_printf_format(format, 0, NULL) : 0;
    argv = malloc(argc * sizeof(char *));

    if (format != NULL) {
        // get the buffer size needed to print varargs (in characters)
        va_start(args, format);
        int length = 1 + vsnprintf(NULL, 0, format, args);
        buf = malloc(length * sizeof(char));
        va_end(args);

        // print varargs on buffer
        va_start(args, format);
        vsnprintf(buf, length, format, args);
        va_end(args);
    }

    // argv[0] must be equal to executable name
    argv[i++] = executable;

    // set varargs arguments
    char *delim = " ";
    char *ptr = strtok(buf, delim);
    while (ptr != NULL) {
        argv[i++] = ptr;
        ptr = strtok(NULL, delim);
    }

    // argv must be terminated by NULL
    argv[i++] = NULL;
    return argv;
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

