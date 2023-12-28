#ifndef MODEL_H
#define MODEL_H

#include "../ipc.h"
#include "../config.h"
#include "../console/console.h"

#if defined(ATTIVATORE) || defined(ATOMO)
#include "../lifo/lifo.h"
#endif

enum Status {
    RUNNING,
    TIMEOUT,
    EXPLODE,
    BLACKOUT,
    MELTDOWN
};

struct Stats {
    enum Status status;
    long energy;
    int n_atoms;
    int n_wastes;
};

struct Model {
    struct Config *config;
    struct Stats *stats;
    struct Ipc *ipc;
#if defined(ATTIVATORE) || defined(ATOMO)
    struct Lifo *lifo;
#endif
};

void init_model(void *shmaddr);

#endif