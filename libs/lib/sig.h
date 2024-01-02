#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

int set_sighandler(int signal, void (*handler)(int));

#endif