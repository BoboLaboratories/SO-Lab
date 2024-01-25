#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "lib/sig.h"
#include "lib/console.h"

sig_atomic_t sig = -1;

static void default_handler(int signum) {
    sig = signum;
}

static void sigterm_handler() {
    exit(EXIT_SUCCESS);
}

// normally set a signal handler
int sig_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    return sigaction(signal, &sa, NULL);
}

// its only intended usage is through its corresponding macro `sig_setup`
void sig_setup_(sigset_t *mask, sigset_t *complementary, int signums, ...) {
    // prepare an empty mask
    sigemptyset(mask);

    // resets every process signal mask regardless of its parent
    sigprocmask(SIG_SETMASK, mask, NULL);

    // prepare a complementary mask if needed
    if (complementary != NULL) {
        sigfillset(complementary);
    }

    va_list args;
    va_start(args, signums);

    int signum = signums;
    while (signum != -1) {
        // remove the current signal from the complementary mask
        if (complementary != NULL) {
            sigdelset(complementary, signum);
        }

        // add the current signal to the mask
        sigaddset(mask, signum);

        // set the handler for the current signal
        sig_handler(signum, &default_handler);

        // get the next signal
        signum = va_arg(args, int);
    }
    va_end(args);

    // automatically handle SIGTERM
    sigdelset(mask, SIGTERM);
    if (complementary != NULL) {
        sigdelset(complementary, SIGTERM);
    }
    sig_handler(SIGTERM, &sigterm_handler);
}

// its only intended usage is through its corresponding macros `mask` and `unmask`
void sig_set_mask_(int how, int signums, ...) {
    if (how != SIG_BLOCK && how != SIG_UNBLOCK) {
        print(E, "Invalid operation on sig_set_mask(%d, ...), please use corresponding macros.\n", how);
        return;
    }

    // prepare an empty mask
    sigset_t mask;
    sigemptyset(&mask);

    va_list args;
    va_start(args, signums);

    // add each of the varargs signals to the mask
    int signum = signums;
    while (signum != -1) {
        sigaddset(&mask, signum);
        signum = va_arg(args, int);
    }
    va_end(args);

    // set the mask
    sigprocmask(how, &mask, NULL);
}