#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <printf.h>
#include <malloc.h>


//void fork_execve(int nchildren, char *executable, char *format, ...) {
//    size_t argc = 0;
//    va_list args;
//    char buf[32];
//    char *str = buf;
//
//    if (format != NULL) {
//        // print varargs to buffer
//        va_start(args, format);
//        argc = parse_printf_format(format, 0, NULL);
//        int length = vsnprintf(buf, sizeof(buf), format, args); // characters used to print varargs (with spaces)
//        va_end(args);
//
//        // allocate more memory if buffer was too small
//        if ((unsigned) length >= sizeof(buf)) {
//            str = malloc((length + 1) * sizeof(char));
//            va_start(args, format);
//            vsnprintf(str, length + 1, format, args);
//            va_end(args);
//        }
//    }
//
//    char **argv = malloc((argc + 2) * sizeof(char *));
//    int i = 0;
//
//    // argv[0] must be equal to executable name
//    argv[i++] = executable;
//
//    // prepare actual arguments
//    char *ptr;
//    char *delim = " ";
//    ptr = strtok(str, delim);
//    while (ptr != NULL) {
//        argv[i++] = ptr;
//        ptr = strtok(NULL, delim);
//    }
//
//    // argv[argc] must be equal to NULL
//    argv[argc + 1] = NULL;
//
//    for (size_t j = 0; j < argc + 1; j++) {
//        printf("%s\n", argv[j]);
//    }
//
//    for (int np = 0; np < nchildren; np++) {
//        switch (fork()) {
//            case -1:
//                errno_fail("Could not fork %s.\n", F_INFO, executable);
//                break;
//            case 0:
//                execve(executable, argvc, NULL);
//                errno_fail("Could not execute %s.\n", F_INFO, executable);
//                break;
//            default:
//                break;
//        }
//    }
//
//    // free dynamically allocated memory if needed
//    free(argv);
//    if (str != buf) {
//        free(str);
//    }
//}

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

#define ITC_SIZE ((3 * sizeof(int) + 1) * sizeof(char))

int main() {
    char **argvc = (char **) malloc(4 * sizeof(char *));
    argvc[1] = malloc(ITC_SIZE);
    argvc[2] = malloc(ITC_SIZE);

    argvc[0] = "atomo";
    argvc[3] = NULL;

    sprintf(argvc[1], "%d", 12);
    sprintf(argvc[2], "%d", 1234);
    printf("%s\n", argvc[0]);
    printf("%s\n", argvc[1]);
    printf("%d\n", argvc[3] == NULL);

    free(argvc[1]);
    free(argvc);

}

//int main() {
//    char *buf = NULL;
//    char **argvc = prargs(buf, "atomo", "%d", 1);
//    for (int i = 0; argvc[i]; i++) {
//        printf("%s\n", argvc[i]);
//    }
//    free(argvc);
//    free(buf);
//}
