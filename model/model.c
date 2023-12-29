#include <stdlib.h>

#include "model.h"

#define OFFSET_CONFIG   0
#define OFFSET_STATS    (OFFSET_CONFIG + sizeof(struct Config))
#define OFFSET_IPC      (OFFSET_STATS + sizeof(struct Stats))
#define OFFSET_LIFO     (OFFSET_IPC + sizeof(struct Ipc))

extern struct Model *model;

void attach_model(void *shmaddr) {
    model->config = shmaddr + OFFSET_CONFIG;
    model->stats = shmaddr + OFFSET_STATS;
    model->ipc = shmaddr + OFFSET_IPC;

#if defined(MASTER) || defined(ATOMO) || defined(ATTIVATORE)
    model->lifo = shmaddr + OFFSET_LIFO;
#endif
}

static void cleanup();

void init() {
    model = malloc(sizeof(struct Model));
    model->res = malloc(sizeof(struct Resources));
    if (atexit(&cleanup) != 0) {
        print(E, "Could not register cleanup function at exit.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

}

void cleanup() {
#ifdef MASTER
#endif

    free(model->res);
    free(model);
}