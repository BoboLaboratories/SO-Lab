#ifndef LIBS_SHMEM_H
#define LIBS_SHMEM_H

#include <sys/shm.h>

#include "../console.h"
#include "../util/util.h"

#define SHM_CONFIG_OFFSET   0
#define SHM_STATS_OFFSET    (sizeof(struct Config))
#define SHM_LIFO_OFFSET     (SHM_STATS_OFFSET)

int shmem_create(key_t key, size_t size, int shmflg);
void *shmem_attach(int shmid);
int shmem_detach(void *shmaddr);
int shmem_remove(int shmid);

#endif
