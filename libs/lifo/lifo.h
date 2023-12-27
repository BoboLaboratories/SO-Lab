#ifndef LIBS_LIFO_H
#define LIBS_LIFO_H

#include <sys/types.h>

#define LIFO_PAGE_SIZE 100

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