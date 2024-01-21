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

static void signal_handler(int signum) {
    sig = signum;
}

static sigset_t va_to_mask(int set_handler, int signums, va_list args) {
    sigset_t mask;
    sigemptyset(&mask);

    int signum = signums;
    do {
        sigaddset(&mask, signum);
        if (set_handler) {
            sig_handler(signum, &signal_handler);
        }
    } while ((signum = va_arg(args, int)) != -1);

    return mask;
}

void sig_handle(int signums, ...) {
    va_list args;
    va_start(args, signums);
    signals = va_to_mask(1, signums, args);
    va_end(args);
}

int sig_is_handled(int signum) {
    return sigismember(&signals, sig);
}

void sig_set_mask(int how, int signums, ...) {
    if (how != SIG_BLOCK && how != SIG_UNBLOCK) {
        print(E, "Invalid operation on sig_set_mask(%d, ...), please use corresponding macros.\n", how);
        return;
    }

    va_list args;
    va_start(args, signums);
    sigset_t mask = va_to_mask(0, signums, args);
    sigprocmask(how, &mask, NULL);
    va_end(args);
}

int sig_reset(int result) {
    sig = -1;
    return result;
}