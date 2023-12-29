#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "libs/util/util.h"

/*
        Global [running]                                                computata dal master
        Total
Atoms   13534                       n_atoms + n_wastes                  computata dal master
Waste   3425                        n_wastes                            incrementato dagli atomi quando diventano scoria
Fission 934                         n_fissions                          incrementato dagli atomi stessi quando si scindono
Att     230                         n_activations                       incrementato dall'attivatore, a priori dell'eventuale inibizione

        Inhibitor [on]                                                  test su semafoto SEM_INHIBITOR_ON
        Total
Fission 234                         n_activations - n_fissions          computata dal master
Energy  4345                        inhibited_energy                    incrementata dall'inibitore
Waste   342                         ???                                 ???

        Total    Remaining
Time    87s      50s                                                    computata dal master

        Total    Used   Free
Energy  1234     1204   30                                              computata dal master
 */

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


int main(int argc, char *argv[]) {
    // informazione privata del master
    long remaining_sim_duration;
}