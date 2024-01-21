#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

#define mask(...)   set_mask_(SIG_BLOCK, __VA_ARGS__, -1)
#define unmask(...) set_mask_(SIG_UNBLOCK, __VA_ARGS__, -1)

int sig_handler(int signal, void (*handler)(int));
void sig_handle(int signums, ...);
void set_mask_(int how, int signums, ...);
int sig_is_handled(int signum);
int sig_reset(int result);

#endif