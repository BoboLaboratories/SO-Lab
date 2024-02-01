#include <time.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <printf.h>
#include <sys/wait.h>

#include "lib/util.h"
#include "lib/console.h"


// =========================================
//              Miscellaneous
// =========================================

timer_t timer_start(long nanos) {
    timer_t timerid;
    timer_create(CLOCK_REALTIME, NULL, &timerid);

    struct itimerspec spec;
    spec.it_value.tv_sec = nanos / (long) 1e9;
    spec.it_value.tv_nsec = nanos % (long) 1e9;
    spec.it_interval = spec.it_value;

    timer_settime(CLOCK_REALTIME, 0, &spec, NULL);

    return timerid;
}

static int srand_done = 0;

int rand_between(int min, int max) {
    if (!srand_done) {
        srand(getpid());
        srand_done = 1;
    }
    return rand() % (max + 1 - min) + min;
}


// =========================================
//             Parsing numbers
// =========================================

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


// =========================================
//    Argument passing and fork utilities
// =========================================

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
    if (argv != NULL) {
        free(argv);
    }
    if (buf != NULL) {
        free(buf);
    }
}

pid_t fork_execv(char **argv) {
    pid_t pid = fork();
    if (pid == 0) {
        execv(argv[0], argv);
        print(E, "Could not execute %s.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    return pid;
}

void wait_children() {
    while (wait(NULL) != -1)
        ;

    if (errno != ECHILD) {
        print(E, "Could not wait for all child processes.\n");
    }
}


// =========================================
//             Temporary files
// =========================================

static const char **tmpfiles = NULL;
int atexit_registered = 0;
int size = 0;

void addtmpfile(const char *pathname) {
    tmpfiles = reallocarray(tmpfiles, ++size, sizeof(char *));
    tmpfiles[size - 1] = pathname;

    if (!atexit_registered) {
        if (atexit(&rmtmpfiles) != 0) {
            print(W, "Could not register temporary file(s) removal at exit.\n");
        } else {
            atexit_registered = 1;
        }
    }
}

void rmtmpfiles() {
    if (tmpfiles != NULL) {
        for (int i = 0; i < size; i++) {
            if (unlink(tmpfiles[i]) == -1) {
                print(E, "Could not delete temporary file (%s).\n", tmpfiles[i]);
            }
        }

        free(tmpfiles);
    }
}