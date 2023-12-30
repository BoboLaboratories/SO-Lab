#ifndef LIBS_LIFO_H
#define LIBS_LIFO_H

#include <sys/types.h>

struct Lifo {
    int size;
    pid_t *ptr;
};

pid_t pop();
void push(pid_t pid);

#endif