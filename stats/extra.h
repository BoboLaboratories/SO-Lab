enum Status {
    RUNNING,
    TIMEOUT,
    EXPLODE,
    BLACKOUT,
    MELTDOWN
};

struct Stats {
    long curr_energy;
    long used_energy;
    long inhibited_energy;
    int n_atoms;
    int n_wastes;
    int n_fissions;
    int n_activations;
};