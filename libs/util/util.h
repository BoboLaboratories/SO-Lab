#ifndef UTIL_H
#define UTIL_H

// TODO Perché?
#define INT_N_CHARS ((3 * sizeof(int) + 1) * sizeof(char))

void nano_sleep(long nanos);
int parse_long(char *raw, long *dest);
void prepare_argv(char *argv[], char buf[], char *executable, int shmid);
int fork_execve(int children, char *executable, char *format, ...);

#endif