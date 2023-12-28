#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

#define RESET       "\e[0m"
#define RED         "\e[0;31m"
#define BLUE        "\e[0;34m"
#define YELLOW      "\e[0;33m"

#define BOLD_RED    "\e[1;31m"
#define BOLD_BLUE   "\e[1;34m"
#define BOLD_YELLOW "\e[1;33m"

#define M_PREFIX    "      "

#define I   0,     NULL,       -1
#define E   1, __FILE__, __LINE__
#define D   2, __FILE__, __LINE__
#define W   3, __FILE__, __LINE__

static const char *prefixes[] = {
    RESET       " INFO " RESET,
    BOLD_RED    "ERROR " RED,
    BOLD_BLUE   "DEBUG " BLUE,
    BOLD_YELLOW " WARN " YELLOW
};

void print(int mode, char *file, int line, char *format, ...) {
    FILE *fd = stdout;
    if (mode == 1) {
        fd = stderr;
    }

    fprintf(fd, "%s", prefixes[mode]);
    if (mode == 1 && errno != 0) {
        fprintf(fd, "errno %d: %s (%s:%d, pid: %5d).\n" M_PREFIX, errno, strerror(errno), file, line, getpid());
    }

    va_list args;
    va_start(args, format);
    vfprintf(fd, format, args);
    va_end(args);
}

int main(int argc, char *argv[]) {
    errno = 16;
    print(E, "Could not attach shared memery (%d).\n", 1456);
    errno = 0;

    print(I, "%d atoms were forked.\n", 57);

    print(D, "Stami ha un tot di neuroni (n=%d).\n", 7);
    print(W, "Could not center Willy today.\n");

    print(E, "Could not attach shared memery (%d).\n", 1456);
}