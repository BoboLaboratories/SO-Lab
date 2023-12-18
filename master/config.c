#include <stdio.h>
#include <stdlib.h>

#include "../libs/console.h"
#include "../libs/ipc/ipc.h"

static const char *variables[9] = {
    "SIM_DURATION",

    "N_ATOMI_INIT",
    "N_NUOVI_ATOMI",

    "MIN_N_ATOMICO",
    "N_ATOM_MAX",

    "STEP_ATTIVATORE",
    "STEP_ALIMENTAZIONE",

    "ENERGY_DEMAND",
    "ENERGY_EXPLODE_THRESHOLD"
};

int parse_long(char *raw, long *dest) {
    char *endptr;
    errno = 0;
    *dest = strtol(raw, &endptr, 10);
    return !(
            errno == ERANGE     // overflow
            || raw == endptr    // no conversion (no characters read)
            || *endptr          // extra characters at the end
    );
}

void load_config() {
    extern struct Model *model;

    int len = sizeof(variables) / sizeof(variables[0]);
    long *addr = (long *) model->config;
    char *raw;

    for (int i = 0; i < len; i++) {
        if ((raw = getenv(variables[i])) == NULL) {
            fail("Configuration environment variable %s is not set.\n", F_INFO, variables[i]);
        }
        if (!parse_long(raw, addr + i)) {
            fail("Bad number format (%s) while parsing environment variable %s.\n", F_INFO, raw, variables[i]);
        }
        if (i != 0 && *(addr + i) <= 0) {
            // only SIM_DURATION (i == 0) may be equal to 0
            // any other config parameter must be greater than 0
            fail("Configuration error: %s must be grater or equals than 0.\n", F_INFO, variables[i]);
        }
    }

    // SIM_DURATION
    if (*(addr + 0) < 0) {
        fail("Configuration error: SIM_DURATION must be grater than or equals to 0.\n", F_INFO);
    }

    // N_ATOM_MAX <= MIN_N_ATOMICO
    if (*(addr + 4) <= *(addr + 3)) {
        fail("Configuration error: N_ATOM_MAX must be greater than MIN_N_ATOMICO.\n", F_INFO);
    }

    // TODO ENERGY_DEMAND e ENERGY_EXPLODE_THRESHOLD
}