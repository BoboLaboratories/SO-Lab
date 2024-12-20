#include <stdio.h>
#include <stdlib.h>

#include "model.h"
#include "lib/util.h"
#include "lib/console.h"

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

int load_config() {
    extern struct Model *model;

    int len = sizeof(variables) / sizeof(variables[0]);
    int err = 0;

    long *addr = (long *) model->config;
    char *raw;

    for (int i = 0; i < len; i++) {
        if ((raw = getenv(variables[i])) == NULL) {
            print(E, "Configuration environment variable %s is not set.\n", variables[i]);
            err = 1;
        }
        if (parse_long(raw, addr + i) == -1) {
            print(E, "Bad number format (%s) while parsing environment variable %s.\n", raw, variables[i]);
            err = 1;
        }
        if (*(addr + i) <= 0) {
            print(E, "Configuration error: %s must be grater or equals than 0.\n", variables[i]);
            err = 1;
        }
    }

    // SIM_DURATION
    if (*(addr + 0) < 1) {
        print(E, "Configuration error: SIM_DURATION must be grater than 0.\n");
        err = 1;
    }

    // MIN_N_ATOMICO >= 1
    if (*(addr + 3) <= 1) {
        print(E, "Configuration error: MIN_N_ATOMICO must be grater than 1.\n");
        err = 1;
    }

    // MIN_N_ATOMICO < N_ATOM_MAX
    if (*(addr + 3) > *(addr + 4)) {
        print(E, "Configuration error: N_ATOM_MAX must be greater than MIN_N_ATOMICO.\n");
        err = 1;
    }

    return err == 1 ? -1 : 0;
}