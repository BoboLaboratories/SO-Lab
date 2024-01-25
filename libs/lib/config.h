#ifndef CONFIG_H
#define CONFIG_H

#define SIM_DURATION             (model->config->sim_duration)

#define N_ATOMI_INIT             (model->config->n_atomi_init)
#define N_NUOVI_ATOMI            (model->config->n_nuovi_atomi)

#define N_ATOM_MAX               (model->config->n_atom_max)
#define MIN_N_ATOMICO            (model->config->min_n_atomico)

#define STEP_ATTIVATORE          (model->config->step_attivatore)
#define STEP_ALIMENTAZIONE       (model->config->step_alimentazione)

#define ENERGY_DEMAND            (model->config->energy_demand)
#define ENERGY_EXPLODE_THRESHOLD (model->config->energy_explode_threshold)


struct Config {
    const unsigned long sim_duration;

    const unsigned long n_atomi_init;
    const unsigned long n_nuovi_atomi;

    const unsigned long n_atom_max;
    const unsigned long min_n_atomico;

    const unsigned long step_attivatore;
    const unsigned long step_alimentazione;

    const unsigned long energy_demand;
    const unsigned long energy_explode_threshold;
};

#endif