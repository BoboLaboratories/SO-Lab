#ifndef UTIL_H
#define UTIL_H

// TODO Perché?
#define INT_N_CHARS ((3 * sizeof(int) + 1) * sizeof(char))

int parse_long(char *raw, long *dest);
void prepare_argv(char *argv[], char buf[], char *executable, int shmid);

#endif