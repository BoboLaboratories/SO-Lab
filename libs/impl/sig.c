#include <string.h>
#include <stdarg.h>

#include "lib/sig.h"
#include "lib/console.h"

sig_atomic_t sig = -1;
sigset_t signals;

int sig_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    return sigaction(signal, &sa, NULL);
}

static void default_handler(int signum) {
    sig = signum;
//#ifdef ATOMO
//    if (sig == 12)
//        print(D, "Recieved SIGWAST: %d\n", getpid());
//#endif
}

static sigset_t va_to_mask(int set_handler, void (*handler)(int), int signums, va_list args) {
    sigset_t mask;
    sigemptyset(&mask);

    int signum = signums;
    while (signum != -1) {
        sigaddset(&mask, signum);
        if (set_handler) {
            sig_handler(signum, handler);
        }
        signum = va_arg(args, int);
    }

    return mask;
}

void sig_set_handler_(void (*handler)(int), int signums, ...) {
    va_list args;
    if (handler == NULL) {
        handler = &default_handler;
    }
    va_start(args, signums);
    signals = va_to_mask(1, handler, signums, args);
    va_end(args);
}

int sig_is_handled(int signum) {
    return sigismember(&signals, signum) == 1;
}

void sig_set_mask_(int how, int signums, ...) {
    if (how != SIG_BLOCK && how != SIG_UNBLOCK) {
        print(E, "Invalid operation on sig_set_mask(%d, ...), please use corresponding macros.\n", how);
        return;
    }

    va_list args;
    va_start(args, signums);
    sigset_t mask = va_to_mask(0, NULL, signums, args);
    sigprocmask(how, &mask, NULL);
    va_end(args);
}

int sig_reset(int result) {
    sig = -1;
    return result;
}