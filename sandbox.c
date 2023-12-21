#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <printf.h>
#include <malloc.h>

void fork_execve(int nchildren, char *executable, char *format, ...) {
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

    // prepare actual arguments
    char *ptr;
    char *delim = " ";
    ptr = strtok(str, delim);
    while (ptr != NULL) {
        argv[i++] = ptr;
        ptr = strtok(NULL, delim);
    }

    // argv[argc] must be equal to NULL
    argv[argc + 1] = NULL;

    for (size_t j = 0; j < argc + 1; j++) {
        printf("%s\n", argv[j]);
    }

    for (int np = 0; np < nchildren; np++) {
        switch (fork()) {
            case -1:
                errno_fail("Could not fork %s.\n", F_INFO, executable);
                break;
            case 0:
                execve(executable, argvc, NULL);
                errno_fail("Could not execute %s.\n", F_INFO, executable);
                break;
            default:
                break;
        }
    }

    // free dynamically allocated memory if needed
    free(argv);
    if (str != buf) {
        free(str);
    }
}


int main() {
   fork_execve(0, "atomo", "%d %d %s %d", 1, 2, "aiuto", 3);
}
