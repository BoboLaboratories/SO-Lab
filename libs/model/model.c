#include <malloc.h>
#include <stdlib.h>

#include "model.h"
#include "../console/console.h"

#define OFFSET_CONFIG   0
#define OFFSET_STATS    (OFFSET_CONFIG + sizeof(struct Config))
#define OFFSET_IPC      (OFFSET_STATS + sizeof(struct Stats))
#define OFFSET_LIFO     (OFFSET_IPC + sizeof(struct Ipc))

extern struct Model *model;

static void free_model();

void init_model(void *shmaddr) {
    model = malloc(sizeof(struct Model));
    if (atexit(&free_model) != 0) {
        print(W, "Could not register model removal at exit.\n");
    }

    model->config = shmaddr + OFFSET_CONFIG;
    model->stats = shmaddr + OFFSET_STATS;
    model->ipc = shmaddr + OFFSET_IPC;

#if defined(ATOMO) || defined(ATTIVATORE)
    model->lifo = shmaddr + OFFSET_LIFO;
#endif
}

static void free_model() {
    free(model);
}