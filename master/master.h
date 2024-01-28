#ifndef MASTER_H
#define MASTER_H

#define INHIBITOR_FLAG        0
#define INHIBITOR_LOG_ON_FLAG 1

static void sigterm_handler();

static int flags[] = {
        [INHIBITOR_FLAG] = 0,
        [INHIBITOR_LOG_ON_FLAG] = 1
};

enum Status {
    STARTING,
    RUNNING,
    TIMEOUT,
    EXPLODE,
    BLACKOUT,
    MELTDOWN,
    TERMINATED
};

struct SimulationStats {
    int inhibitor_off;
    enum Status status;
    struct Stats stats;
    unsigned long remaining_seconds;
};



#endif