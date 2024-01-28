#ifndef LIBS_SHMEM_H
#define LIBS_SHMEM_H

#include <sys/types.h>

int shmem_create(key_t key, size_t size, int shmflg);
void *shmem_attach(int shmid);
int shmem_detach(void *shmaddr);
int shmem_rmark(int shmid);

#endif