#ifndef MODEL_H
#define MODEL_H

#include "../lib/ipc.h"
#include "../lib/config.h"
#include "../lib/console/console.h"

#if defined(ATTIVATORE) || defined(ATOMO)
#include "../lib/lifo/lifo.h"
#endif

#ifdef MASTER
enum Status {
    STARTING,
    RUNNING,
    TIMEOUT,
    EXPLODE,
    BLACKOUT,
    MELTDOWN
};
#endif

struct Stats {
    long curr_energy;
    long used_energy;
    long inhibited_energy;
    long n_atoms;
    long n_wastes;
    long n_fissions;
    long n_activations;
};

struct Model {
    struct Config *config;
    struct Stats *stats;
    struct Ipc *ipc;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ATOMO)
    struct Lifo *lifo;
#endif
    struct Resources {
        void *shmaddr;
        int shmid;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ALIMENTATORE)
        int fifo_fd;
#endif
    } *res;
};

void init();
void attach_model(void *shmaddr);

#endif