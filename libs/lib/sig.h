#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

#define mask(...)                           sig_set_mask_(SIG_BLOCK, __VA_ARGS__, -1)
#define unmask(...)                         sig_set_mask_(SIG_UNBLOCK, __VA_ARGS__, -1)
#define sig_setup(mask, complementary, ...) sig_setup_(mask, complementary, __VA_ARGS__, -1)
#define sig_handle(handler, ...)    sig_set_handler_(handler, __VA_ARGS__, -1)

int sig_handler(int signal, void (*handler)(int));
//void sig_handle(void (*handler)(int), sigset_t *mask);
void sig_setup_(sigset_t *mask, sigset_t *complementary, int signums, ...);
void sig_set_mask_(int how, int signums, ...);
void sig_set_handler_(void (*handler)(int), int signums, ...);
int sig_is_handled(int signum);
int sig_reset(int result);

#endif