#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
#include <sys/types.h>

// TODO Perché?
// (I)nteger (T)o (C)hars
#define ITC_SIZE ((3 * sizeof(int) + 1) * sizeof(char))

// misc
void nano_sleep(long nanos);

// numerical parsing
int parse_long(char *raw, long *dest);
int parse_int(char *raw, int *dest);

// arguments and fork
void prargs(char *executable, char ***argv, char **buf, int vargs, size_t elemsize);
void frargs(char **argv, char *buf);
pid_t fork_execve(char **argv);
void wait_children();

// tmp files
int mktmpfile(const char *pathname, int flags, mode_t mode);
void addtmpfile(const char *pathname);
void rmtmpfiles();

#endif