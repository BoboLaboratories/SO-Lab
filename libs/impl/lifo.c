#include <string.h>
#include <fcntl.h>

#include "lib/sem.h"
#include "lib/util.h"
#include "lib/lifo.h"
#include "lib/shmem.h"
#include "lib/console.h"

#define POP  0
#define PUSH 1

static void *attach(struct Lifo *lifo, int pushing);
static int acquire(struct Lifo *lifo);
static int release(struct Lifo *lifo);

void lifo_create(struct Lifo *lifo, int segment_length, size_t elem_size, int semid, int sem_num) {
    struct Lifo tmp = {
            .shmid = -1,    // initially, there's no shared memory as there is no data in the lifo
            .length = 0,
            .semid = semid,
            .sem_num = sem_num,
            .segment_length = segment_length,
            .elem_size = elem_size
    };
    memcpy(lifo, &tmp, sizeof(struct Lifo));
}

int lifo_push(struct Lifo *lifo, void *data) {
    int ret = -1;

    if (acquire(lifo) != -1) {
        void *shmaddr;
        if ((shmaddr = attach(lifo, PUSH)) != (void *) -1) {
            size_t offset = lifo->length * lifo->elem_size;
            memcpy(shmaddr + offset, data, lifo->elem_size);
            lifo->length++;
            ret = 0;

            shmem_detach(shmaddr);
        }
        release(lifo);
    }

    return ret;
}

int lifo_pop(struct Lifo *lifo, void *data) {
    int ret = -1;

    if (acquire(lifo) != -1) {
        if (lifo->length > 0) {
            void *shmaddr;
            if ((shmaddr = attach(lifo, POP)) != (void *) -1) {
                lifo->length--;
                size_t offset = lifo->length * lifo->elem_size;
                memcpy(data, shmaddr + offset, lifo->elem_size);
                ret = 0;

                shmem_detach(shmaddr);
            }
        }
        release(lifo);
    }

    return ret;
}

int lifo_delete(struct Lifo *lifo) {
    int ret = -1;

    if (acquire(lifo) != -1) {
        if ((ret = shmem_rmark(lifo->shmid)) == -1) {
            print(E, "Could not delete lifo.\n");
        }
        release(lifo);
    }

    return ret;
}

static int acquire(struct Lifo *lifo) {
    struct sembuf sops;
    sem_buf(&sops, lifo->sem_num, -1, 0);
    int ret = sem_op(lifo->semid, &sops, 1);
    if (ret == -1) {
        print(E, "Could not acquire LIFO semaphore.\n");
    }
    return ret;
}

static int release(struct Lifo *lifo) {
    struct sembuf sops;
    sem_buf(&sops, lifo->sem_num, +1, 0);
    int ret = sem_op(lifo->semid, &sops, 1);
    if (ret == -1) {
        print(E, "Could not release LIFO semaphore.\n");
    }
    return ret;
}

static void swap(struct Lifo *lifo, size_t size, void **dest) {
    int tmp;
    // create a new shared memory segment of the specified size
    if ((tmp = shmem_create(IPC_PRIVATE, size, S_IWUSR | S_IRUSR | IPC_CREAT)) != -1) {

        // if a segment was already present
        if (lifo->shmid != -1) {
            // attach the old segment
            void *src = shmem_attach(lifo->shmid);

            // attach the new segment
            dest = shmem_attach(tmp);

            // copy data from the old to the new segment
            memcpy(dest, src, lifo->length * lifo->elem_size);

            // mark the old segment for removal
            shmem_rmark(lifo->shmid);

            // detach from the old segment (will also trigger removal)
            shmem_detach(src);
        }

        // actually swap the shared memory id
        lifo->shmid = tmp;
    }
}

static void *attach(struct Lifo *lifo, int pushing) {
    // current number of segments that make up the
    // shared memory (as specified in lifo->segment_length)
    int n_segments = ceil(lifo->length / lifo->segment_length);

    // current number of free slots
    int free_slots = n_segments * lifo->segment_length - lifo->length;

    // assume no new segment is needed
    int new_n_segments = n_segments;

    // pointer to the shared memory segment
    void *dest = NULL;

    // check if shared memory should be resized
    if (pushing && free_slots == 0) {
        new_n_segments++;
    } else if (free_slots >= 2 * lifo->segment_length) {
        // segment length is multiplied by 2 to keep some room for the next operations
        // shared memory is only shrank if there's more than one completely empty
        new_n_segments--;
    }

    // if shared memory size has changed, perform the swap
    if (n_segments != new_n_segments) {
        size_t size = new_n_segments * lifo->segment_length * lifo->elem_size;
        swap(lifo, size, &dest);
    }

    // if shared memory was not swapped, attach to the current one
    if (dest == NULL) {
        dest = shmem_attach(lifo->shmid);
    }

    return dest;
}