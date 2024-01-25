#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

#define mask(...)                           sig_set_mask_(SIG_BLOCK, __VA_ARGS__, -1)
#define unmask(...)                         sig_set_mask_(SIG_UNBLOCK, __VA_ARGS__, -1)
#define sig_setup(mask, complementary, ...) sig_setup_(mask, complementary, __VA_ARGS__, -1)

int sig_handler(int signal, void (*handler)(int));
void sig_setup_(sigset_t *mask, sigset_t *complementary, int signums, ...);
void sig_set_mask_(int how, int signums, ...);

#endif