#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "lib/console.h"

#define RESET       "\e[0m"
#define RED         "\e[0;31m"
#define BLUE        "\e[0;34m"
#define YELLOW      "\e[0;33m"

#define BOLD_RED    "\e[1;31m"
#define BOLD_BLUE   "\e[1;34m"
#define BOLD_YELLOW "\e[1;33m"

#define PREFIX    "      "

static const char *prefixes[] = {
        RESET       " INFO " RESET,
        BOLD_RED    "ERROR " RED,
        BOLD_BLUE   "DEBUG " BLUE,
        BOLD_YELLOW " WARN " YELLOW
};

void print(int mode, char *file, int line, char *format, ...) {
    int errno_bak = errno;
    FILE *fd = stdout;
    if (mode == 1) {
        fd = stderr;
    }

    pid_t pid = getpid();
#ifdef DEBUG
    fprintf(fd, "%s%d ", RESET, pid);
#endif
    fprintf(fd, "%s", prefixes[mode]);
    if (mode == 1 && errno != 0) {
        fprintf(fd, "errno %d: %s (%s:%d, pid: %5d).\n" RESET "%d " RED PREFIX, errno_bak, strerror(errno_bak), file, line, pid, pid);
    }

    va_list args;
    va_start(args, format);
    vfprintf(fd, format, args);
    fprintf(fd, RESET);
    va_end(args);

    errno = errno_bak;
}