#include <stdlib.h>
#include <sys/shm.h>
#include <stdio.h>

#include "lifo.h"

static struct Page *curr = NULL;
static struct Page *buf = NULL;
static pid_t *ptr = NULL;
static int size = 0;
static int semid;

void init_lifo(int sem) {
    semid = sem;
}

pid_t pop() {
    if (size == 0) {
        return -1;
    }

    pid_t pid = *ptr;
    size--;

    printf("Reading %d\n", pid);

    if (size > 0 && ptr == curr->addr) {
        if (buf != NULL) {
            if (shmdt(buf->addr) == -1) {
                printf("Error detaching buf shmem.\n");
            } else {
                printf("Detached buf shmem.\n");
            }

            if (shmctl(buf->shmid, IPC_RMID, NULL) == -1) {
                printf("Error deleting buf shmem.\n");
            } else {
                printf("Deleted buf shmem.\n");
            }
        }

        if (curr->prev != NULL) {
            buf = curr;
            curr = curr->prev;

            curr->addr = shmat(curr->shmid, NULL, 0);
            if (curr->addr == (void *) -1) {
                printf("Error attaching previous page.\n");
            } else {
                printf("Attached previous page.\n");
            }

            ptr = curr->addr + LIFO_PAGE_SIZE - sizeof(pid_t);
        }
    } else {
        ptr -= sizeof(pid_t);
    }

    printf("Ptr: %p, end on: %p\n", ptr, curr->addr);
    printf("Done reading %d\n\n", pid);

    return pid;
}

void push(pid_t pid) {
    printf("Writing %d\n", pid);

    if (curr == NULL || ptr == curr->addr + LIFO_PAGE_SIZE - sizeof(pid_t)) {
        printf("Creating new page.\n");
        if (buf == NULL) {
            buf = malloc(sizeof(struct Page));

            buf->shmid = shmget(IPC_PRIVATE, LIFO_PAGE_SIZE, 0666 | IPC_CREAT);
            if (buf->shmid == -1) {
                // TODO boh
                printf("Error while creating shmem.\n");
            }
        }

        if (curr != NULL) {
            if (shmdt(curr->addr) == -1) {
                printf("Error detaching while changing page\n");
            } else {
                printf("Detached previous page.\n");
            }
        }

        buf->prev = curr;
        curr = buf;
        buf = NULL;

        curr->addr = shmat(curr->shmid, NULL, 0);
        if (curr->addr == (void *) -1) {
            // TODO delete page
            printf("Error attaching next page.\n");
        } else {
            printf("Attached next page.\n");
        }

        ptr = curr->addr;
    } else {
        ptr += sizeof(pid_t);
    }

    *ptr = pid;
    size++;
    //printf("pointer: %p of max: %p\n", ptr, curr->addr + LIFO_PAGE_SIZE - sizeof (pid_t));
    printf("Done writing %d, space left: %lu\n\n", pid, ((curr->addr + LIFO_PAGE_SIZE - sizeof (pid_t)) - ptr) / sizeof(pid_t));
}