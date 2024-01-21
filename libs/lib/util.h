#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
#include <sys/types.h>

// TODO Perché?
// (I)nteger (T)o (C)hars
#define CHARS_PER_INT   (3 * sizeof(int) + 1)
#define ITC_SIZE        (CHARS_PER_INT * sizeof(char))

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

// misc
timer_t timer_start(long nanos);
int rand_between(int min, int max);

// numerical parsing
int parse_long(char *raw, long *dest);
int parse_int(char *raw, int *dest);

// arguments and fork
void prargs(char *executable, char ***argv, char **buf, int vargs, size_t elemsize);
void frargs(char **argv, char *buf);
pid_t fork_execv(char **argv);
void wait_children();

// tmp files
int mktmpfile(const char *pathname, int flags, mode_t mode);
void addtmpfile(const char *pathname);
void rmtmpfiles();

#endif