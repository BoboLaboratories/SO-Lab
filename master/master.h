#ifndef MASTER_H
#define MASTER_H

#define INHIBITOR_FLAG  0

static void sigterm_handler();

static int flags[1] = {
        [INHIBITOR_FLAG] = 0
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

#endif
