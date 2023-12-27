#ifndef MODEL_H
#define MODEL_H

#include "../config.h"

enum Component {
    MASTER,
    ATOMO,
    ATTIVATORE,
    ALIMENTATORE,
    INIBITORE
};

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

void init_model(void *shmaddr);

#endif