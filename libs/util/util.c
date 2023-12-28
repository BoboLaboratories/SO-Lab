#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <printf.h>
#include <sys/wait.h>

#include "util.h"
#include "../console/console.h"

void nano_sleep(long nanos) {
    extern sig_atomic_t *interrupted;

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
        print(E, "Could not execute %s.\n", argv[0]);
        kill(getppid(), SIGTERM);     // TODO test if working as expected
    }
    return pid;
}

void wait_children() {
    while (wait(NULL) != -1)
        continue;

    if (errno != ECHILD) {
        // TODO capire come e se gestire questo errore
    }
}

static const char **tmpfiles;
int atexit_registered = 0;
int size = 0;

int mktmpfile(const char *pathname, int flags, mode_t mode) {
    int fd = open(pathname, flags, mode);
    if (fd == -1) {
        print(E, "Could not create temporary file (%s).\n", pathname);
        exit(EXIT_FAILURE);
    } else {
#ifdef DEBUG
        print(D, "Created temporary file (%s).\n", pathname);
#endif
        addtmpfile(pathname);
        if (!atexit_registered) {
            if (atexit(&rmtmpfiles) != 0) {
                print(W, "Could not register temporary file(s) removal at exit.\n");
            } else {
                atexit_registered = 1;
#ifdef DEBUG
                print(D, "Registered temporary file(s) cleanup at exit.\n", pathname);
#endif
            }
        }
    }

    return fd;
}

void addtmpfile(const char *pathname) {
    tmpfiles = reallocarray(tmpfiles, size++, sizeof(char *));
    tmpfiles[size - 1] = pathname;
}

void rmtmpfiles() {
    for (int i = 0; i < size; i++) {
        if (unlink(tmpfiles[i]) == -1) {
            print(E, "Could not delete temporary file (%s).\n", tmpfiles[1]);
        }
#ifdef DEBUG
        else {
            print(D, "Deleted temporary file (%s).\n", tmpfiles[1]);
        }
#endif
    }

    free(tmpfiles);
}