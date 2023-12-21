#ifndef UTIL_H
#define UTIL_H

#include <signal.h>

// TODO Perché?
#define INT_N_CHARS ((3 * sizeof(int) + 1) * sizeof(char))

void nano_sleep(long nanos, sig_atomic_t *interrupted);
int parse_long(char *raw, long *dest);
pid_t fork_execve(char **argv);
char **prargs(char *buf, char *executable, char *format, ...);

#endif