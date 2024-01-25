#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "lib/console.h"

#define EMPTY_PREFIX "      "

static const char *prefixes[] = {
        RESET       " INFO " RESET,
        BOLD_RED    "ERROR " RED,
        BOLD_BLUE   "DEBUG " BLUE,
        BOLD_YELLOW " WARN " YELLOW
};

void print(int mode, char *file, int line, char *format, ...) {
    // backup errno
    int errno_bak = errno;

    // if error output mode, print to stderr
    FILE *fd = stdout;
    if (mode == 1) {
        fd = stderr;
    }

    pid_t pid = getpid();
    fprintf(fd, "%s", prefixes[mode]);

    if (mode == 1 && errno != 0) {
        // if error output mode is set and errno is set
        // print errno and other debug information
        fprintf(fd, "errno %d: %s (%s:%d, pid: %5d).\n" RESET "%d " RED EMPTY_PREFIX, errno_bak, strerror(errno_bak), file, line, pid, pid);
    }

    // print the actual output message
    va_list args;
    va_start(args, format);
    vfprintf(fd, format, args);
    fprintf(fd, RESET);
    va_end(args);

    // restore errno
    errno = errno_bak;
}