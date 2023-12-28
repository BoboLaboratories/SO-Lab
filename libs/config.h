#ifndef CONFIG_H
#define CONFIG_H

#define SIM_DURATION             (config->sim_duration)

#define N_ATOMI_INIT             (config->n_atomi_init)
#define N_NUOVI_ATOMI            (config->n_nuovi_atomi)

#define N_ATOM_MAX               (config->n_atom_max)
#define MIN_N_ATOMICO            (config->min_n_atomico)

#define STEP_ATTIVATORE          (config->step_attivatore)
#define STEP_ALIMENTAZIONE       (config->step_alimentazione)

#define ENERGY_DEMAND            (config->energy_demand)
#define ENERGY_EXPLODE_THRESHOLD (config->energy_explode_threshold)


struct Config {
    const long sim_duration;

    const long n_atomi_init;
    const long n_nuovi_atomi;

    const long n_atom_max;
    const long min_n_atomico;

    const long step_attivatore;
    const long step_alimentazione;

    const long energy_demand;
    const long energy_explode_threshold;
};

#endif