#ifndef LIBS_LIFO_H
#define LIBS_LIFO_H

#include <sys/types.h>

struct Lifo {
    int shmid;
    int length;
    const int semid;
    const int sem_num;
    const int segment_length;
    const size_t elem_size;
};

void mklifo(struct Lifo *lifo, int segment_length, size_t elem_size, int semid, int sem_num);
void lifo_push(struct Lifo *lifo, void *data);
void lifo_pop(struct Lifo *lifo, void *data);
int rmlifo(struct Lifo *lifo);

#endif