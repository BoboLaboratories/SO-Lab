#include <stdlib.h>

#include "model.h"
#include "lib/sem.h"

#define OFFSET_CONFIG   0
#define OFFSET_STATS    (OFFSET_CONFIG + sizeof(struct Config))
#define OFFSET_IPC      (OFFSET_STATS + sizeof(struct Stats))
#define OFFSET_LIFO     (OFFSET_IPC + sizeof(struct Ipc))

struct Model *model = NULL;

extern void cleanup();
static void cleanup_model();
static void signal_handler(int signum);

void init() {
#ifdef DEBUG
    print(D, "Init\n");
    setbuf(stdout, NULL);
#endif

    model = malloc(sizeof(struct Model));
    model->res = malloc(sizeof(struct Resources));

    model->res->shmid = -1;
    model->res->shmaddr = (void *) -1;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ATOMO)
    model->res->fifo_fd = -1;
#endif

    if (atexit(&cleanup_model) != 0) {
        print(E, "Could not register model cleanup function at exit.\n");
        cleanup();
        cleanup_model();
        exit(EXIT_FAILURE);
    }
    if (atexit(&cleanup) != 0) {
        print(E, "Could not register cleanup function at exit.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }
}

void attach_model(void *shmaddr) {
    model->config = shmaddr + OFFSET_CONFIG;
    model->stats = shmaddr + OFFSET_STATS;
    model->ipc = shmaddr + OFFSET_IPC;

#if defined(MASTER) || defined(ATOMO) || defined(ATTIVATORE) || defined(INIBITORE)
    model->lifo = shmaddr + OFFSET_LIFO;
#endif
}

static void cleanup_model() {
    free(model->res);
    free(model);
}

#if defined(ATOMO) || defined(INIBITORE)
int end_activation_cycle() {
    struct sembuf sops;
    sem_buf(&sops, SEM_MASTER, +1, 0);
    if (sem_op(model->ipc->semid, &sops, 1) == -1) {
        print(E, "Could not release master semaphore.\n");
    }

    sem_buf(&sops, SEM_ATTIVATORE, +1, 0);
    if (sem_op(model->ipc->semid, &sops, 1) == -1) {
        print(E, "Could not increase activator semaphore.\n");
    }
}
#endif