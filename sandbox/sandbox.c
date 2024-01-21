#include <stdio.h>
#include <stdarg.h>
#include "lib/sig.h"

extern struct Model *model;

static sigset_t va_to_mask(int signums, va_list args) {
    sigset_t mask;
    sigemptyset(&mask);

    sigaddset(&mask, signums);

    int sig;
    while ((sig = va_arg(args, int)) != -1) {
        sigaddset(&mask, sig);
    }

    return mask;
}

void set_meaningful_signals(sigset_t *sigset, int signums, ...) {
    va_list args;
    va_start(args, signums);
    *sigset = va_to_mask(signums, args);
    va_end(args);
}

int main(int argc, char *argv[]) {
    sigset_t mask;
    sigemptyset(&mask);

    set_meaningful_signals(&mask, SIGUSR1, SIGTERM, SIGUSR2, -1);

    if (sigismember(&mask, SIGUSR1)) {
        printf("SIGUSR1\n");
    }

    if (sigismember(&mask, SIGTERM)) {
        printf("SIGTERM!\n");
    }
    if (sigismember(&mask, SIGUSR2)) {
        printf("SIGUSER2!\n");
    }
}

void cleanup() {

}

