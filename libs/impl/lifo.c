#include <math.h>
#include <string.h>

#include "lib/lifo.h"
#include "lib/sem.h"
#include "lib/shmem.h"

#define POP  0
#define PUSH 1

static void *attach(struct Lifo *lifo, int pushing);

void mklifo(struct Lifo *lifo, int segment_length, size_t elem_size, int semid, int sem_num) {
    struct Lifo tmp = {
        .shmid = -1,
        .length = 0,
        .semid = semid,
        .sem_num = sem_num,
        .segment_length = segment_length,
        .elem_size = elem_size
    };
    memcpy(lifo, &tmp, sizeof(struct Lifo));
}

void lifo_push(struct Lifo *lifo, void *data) {
    struct sembuf sops;
    sem_buf(&sops, lifo->sem_num, -1, 0);
    sem_op(lifo->semid, &sops, 1);

    void *shmaddr;
    if ((shmaddr = attach(lifo, PUSH)) != (void *) -1) {
        size_t offset = lifo->length * lifo->elem_size;
        memcpy(shmaddr + offset, data, lifo->elem_size);
        lifo->length++;

        shmem_detach(shmaddr);
    }

    sem_buf(&sops, lifo->sem_num, 1, 0);
    sem_op(lifo->semid, &sops, 1);
}

void lifo_pop(struct Lifo *lifo, void *data) {
    struct sembuf sops;
    sem_buf(&sops, lifo->sem_num, -1, 0);
    sem_op(lifo->semid, &sops, 1);

    void *shmaddr;
    if ((shmaddr = attach(lifo, POP)) != (void *) -1) {
        lifo->length--;
        size_t offset = lifo->length * lifo->elem_size;
        memcpy(data, shmaddr + offset, lifo->elem_size);

        shmem_detach(shmaddr);
    }

    sem_buf(&sops, lifo->sem_num, 1, 0);
    sem_op(lifo->semid, &sops, 1);
}

int rmlifo(struct Lifo *lifo) {
    struct sembuf sops;
    sem_buf(&sops, lifo->sem_num, -1, 0);
    sem_op(lifo->semid, &sops, 1);

    int ret = shmem_rmark(lifo->shmid);

    sem_buf(&sops, lifo->sem_num, 1, 0);
    sem_op(lifo->semid, &sops, 1);

    return ret;
}

static void swap(struct Lifo *lifo, size_t size) {
    int tmp;
    if ((tmp = shmem_create(IPC_PRIVATE, size, S_IWUSR | S_IRUSR | IPC_CREAT)) != -1) {
        if (lifo->shmid != -1) {
            shmem_rmark(lifo->shmid);
        }
        lifo->shmid = tmp;
    }
}

static void *attach(struct Lifo *lifo, int pushing) {
    int n_segments = ceil((double) lifo->length / lifo->segment_length);
    int free_slots = n_segments * lifo->segment_length - lifo->length;
    size_t size = -1;

    if (pushing && free_slots == 0) {
        size = (n_segments + 1) * lifo->segment_length * lifo->elem_size;
    } else if (free_slots >= 2 * lifo->segment_length) {
        size = (n_segments - 1) * lifo->segment_length * lifo->elem_size;
    }

    if (size != (size_t) -1) {
        swap(lifo, size);
    }

    return shmem_attach(lifo->shmid);
}