#ifndef UTIL_H
#define UTIL_H

#include <signal.h>

// TODO Perché?
// (I)nteger (T)o (C)hars
#define ITC_SIZE ((3 * sizeof(int) + 1) * sizeof(char))

void nano_sleep(sig_atomic_t *interrupted, long nanos);
int parse_long(char *raw, long *dest);
pid_t fork_execve(char **argv);

#endif