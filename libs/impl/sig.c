#include <string.h>

#include "lib/sig.h"

int set_sighandler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    return sigaction(signal, &sa, NULL);
}