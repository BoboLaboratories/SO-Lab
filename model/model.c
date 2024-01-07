#include <stdlib.h>

#include "lib/sig.h"

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

#if defined(MASTER) || defined(ATOMO) || defined(ATTIVATORE) || defined(INIBITORE)
    model->lifo = shmaddr + OFFSET_LIFO;
#endif
}

extern void cleanup();
static void cleanup_model();

void init() {
    model = malloc(sizeof(struct Model));
    model->res = malloc(sizeof(struct Resources));

    model->res->shmid = -1;
    model->res->shmaddr = (void *) -1;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ALIMENTATORE)
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

    extern void signal_handler();
    set_sighandler(SIGTERM, &signal_handler);
}

static void cleanup_model() {
    free(model->res);
    free(model);
}