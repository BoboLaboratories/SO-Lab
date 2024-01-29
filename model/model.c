#include <stdlib.h>

#include "model.h"
#include "lib/util.h"
#include "lib/shmem.h"

#if defined(ATOMO) || defined(INIBITORE)
#include "lib/sem.h"
#endif

#define OFFSET_CONFIG   0
#define OFFSET_STATS    (OFFSET_CONFIG + sizeof(struct Config))
#define OFFSET_IPC      (OFFSET_STATS + sizeof(struct Stats))
#define OFFSET_LIFO     (OFFSET_IPC + sizeof(struct Ipc))

struct Model *model = NULL;

#if !defined(MASTER)
void attach_model();
#endif

extern void cleanup();
static void cleanup_model();

void init(char *shmid) {
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

    if (shmid != NULL) {
        if (parse_int(shmid, &model->res->shmid) == -1) {
            print(E, "Could not parse shmid (%s).\n", shmid);
            exit(EXIT_FAILURE);
        }

        if ((model->res->shmaddr = shmem_attach(model->res->shmid)) == (void *) -1) {
            exit(EXIT_FAILURE);
        }

        attach_model();
    }
}


void attach_model() {
    model->config = model->res->shmaddr + OFFSET_CONFIG;
    model->stats = model->res->shmaddr + OFFSET_STATS;
    model->ipc = model->res->shmaddr + OFFSET_IPC;

#if defined(MASTER) || defined(ATOMO) || defined(ATTIVATORE) || defined(INIBITORE)
    model->lifo = model->res->shmaddr + OFFSET_LIFO;
#endif
}

static void cleanup_model() {
    free(model->res);
    free(model);
}