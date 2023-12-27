#ifndef LIFO_H
#define LIFO_H

#include <sys/types.h>

#ifndef PID_PER_LIFO_PAGE
#define PID_PER_LIFO_PAGE 2
#endif

#define LIFO_PAGE_SIZE (PID_PER_LIFO_PAGE * sizeof(pid_t))

struct Lifo {
    struct Page *curr;
    struct Page *buf;
    pid_t *ptr;
    int size;
    int semid;
};

struct Page {
    int shmid;
    pid_t *addr;
    struct Page *prev;
};

void init_lifo(int sem);
pid_t pop();
void push(pid_t pid);

#endif