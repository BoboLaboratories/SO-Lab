#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>

#define ITC_SIZE ((3 * sizeof(int) + 1) * sizeof(char))

//void prargs(char *executable, char ***argv, char **buf, int vargs, size_t elemsize) {
//    char **argvt = calloc(vargs + 2, sizeof(char *));
//    char *buft = calloc(vargs, elemsize);
//
//    argvt[0] = executable;
//    for (int i = 0; i < vargs; i++) {
//        argvt[i + 1] = &buft[i * elemsize];
//    }
//    argvt[vargs + 1] = NULL;
//
//    *argv = argvt;
//    *buf = buft;
//}


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

int main() {
    char *buf;
    char **argv;

    prargs("atomo", &argv, &buf, 2, ITC_SIZE);
    sprintf(argv[1], "%d", 123);
    sprintf(argv[2], "%d", -23);

    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);
    printf("%s\n", argv[2]);
    printf("%p\n", argv[3]);

    frargs(argv, buf);

    exit(1);
}