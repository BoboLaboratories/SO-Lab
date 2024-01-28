#ifndef MODEL_H
#define MODEL_H

#include "lib/ipc.h"
#include "lib/config.h"
#include "lib/console.h"

#if defined(MASTER) || defined(ATTIVATORE) || defined(INIBITORE) || defined(ATOMO)
#include "lib/lifo.h"
#endif

struct Stats {
    unsigned long curr_energy;
    unsigned long used_energy;
    unsigned long inhibited_energy;
    unsigned long inhibited_atoms;
    unsigned long n_atoms;
    unsigned long n_wastes;
    unsigned long n_fissions;
    unsigned long n_activations;
};

struct Model {
    // start information in shared memory
    struct Config *config;
    struct Stats *stats;
    struct Ipc *ipc;
#if defined(MASTER) || defined(ATTIVATORE) || defined(INIBITORE) || defined(ATOMO)
    struct Lifo *lifo;
#endif
    // end information in shared memory
    // start local process IPC information
    struct Resources {
        void *shmaddr;
        int shmid;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ATOMO)
        int fifo_fd;
#endif
    } *res;
    // end local process IPC information
};

void init(char *shmid);

#if defined(MASTER)
void attach_model();
#endif


#if defined(ATOMO) || defined(INIBITORE)
int end_activation_cycle();
#endif

#endif