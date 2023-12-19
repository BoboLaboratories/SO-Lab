#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "util.h"

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