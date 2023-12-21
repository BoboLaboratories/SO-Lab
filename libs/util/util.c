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

void nano_sleep(long nanos) {
    struct timespec t;
    t.tv_sec = nanos / 1000000000;
    t.tv_nsec = nanos % 1000000000;
    while (nanosleep(&t, &t) == -1)
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

void prepare_argv(char *argv[], char buf[], char *executable, int shmid) {
    sprintf(buf, "%d", shmid);

    argv[0] = executable;
    argv[1] = buf;
    argv[2] = (char *) 0;
}

int fork_execve(int children, char *executable, char *format, ...) {
    size_t argc = 0;
    va_list args;
    char buf[32];
    char *str = buf;

    if (format != NULL) {
        // print varargs to buffer
        va_start(args, format);
        argc = parse_printf_format(format, 0, NULL);
        int length = vsnprintf(buf, sizeof(buf), format, args); // characters used to print varargs (with spaces)
        va_end(args);

        // allocate more memory if buffer was too small
        if ((unsigned) length >= sizeof(buf)) {
            str = malloc((length + 1) * sizeof(char));
            va_start(args, format);
            vsnprintf(str, length + 1, format, args);
            va_end(args);
        }
    }

    char **argv = malloc((argc + 2) * sizeof(char *));
    int i = 0;

    // argv[0] must be equal to executable name
    argv[i++] = executable;

    // set varargs arguments
    char *ptr;
    char *delim = " ";
    ptr = strtok(str, delim);
    while (ptr != NULL) {
        argv[i++] = ptr;
        ptr = strtok(NULL, delim);
    }

    // argv[argc] must be equal to NULL
    argv[argc + 1] = NULL;

    // fork children processes
    int np;
    int error = 0;
    for (np = 0; !error && np < children; np++) {
        switch (fork()) {
            case -1:
                error = 1;
                break;
            case 0:
                execve(executable, argv, NULL);
                errno_fail("Could not execute %s.\n", F_INFO, executable);
                break;
            default:
                // parent process noop
                break;
        }
    }

    // free dynamically allocated memory (if needed)
    free(argv);
    if (str != buf) {
        free(str);
    }

    return np + 1;
}

