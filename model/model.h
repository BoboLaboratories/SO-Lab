#ifndef MODEL_H
#define MODEL_H

#include "lib/ipc.h"
#include "lib/config.h"
#include "lib/console.h"

#if defined(MASTER) || defined(ATTIVATORE) || defined(INIBITORE) || defined(ATOMO)
#include "lib/lifo.h"
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
#if defined(MASTER) || defined(ATTIVATORE) || defined(INIBITORE) || defined(ATOMO)
    struct Lifo *lifo;
#endif
    struct Resources {
        void *shmaddr;
        int shmid;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ATOMO)
        int fifo_fd;
#endif
    } *res;
};

void init();
void attach_model(void *shmaddr);

#if defined(ATOMO) || defined(INIBITORE)
int end_activation_cycle();
#endif

#endif