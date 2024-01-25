#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
#include <sys/types.h>

// the amount of characters needed to print the largest int for this architecture
#define CHARS_PER_INT   (3 * sizeof(int) + 1)

// (I)nteger (T)o (C)hars
#define ITC_SIZE        (CHARS_PER_INT * sizeof(char))


// avoid importing whole math library for these simple utilities
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define ceil(x) ((x - (int) x) == 0 ? (int) x : (int) x + 1)


// miscellaneous
timer_t timer_start(long nanos);
int rand_between(int min, int max);


// parsing numbers
int parse_long(char *raw, long *dest);
int parse_int(char *raw, int *dest);


// argument passing and fork utilities
void prargs(char *executable, char ***argv, char **buf, int vargs, size_t elemsize);
void frargs(char **argv, char *buf);
pid_t fork_execv(char **argv);
void wait_children();


// temporary files
void addtmpfile(const char *pathname);
void rmtmpfiles();

#endif