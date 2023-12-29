/*
        Global [running]                                                computata dal master
        Total
Atoms   13534                       n_atoms + n_wastes                  computata dal master
Waste   3425                        n_wastes                            incrementato dagli atomi quando diventano scoria
Fission 934                         n_fissions                          incrementato dagli atomi stessi quando si scindono
Att     230                         n_activations                       incrementato dall'attivatore, a priori dell'eventuale inibizione

        Inhibitor [on]                                                  test su semaforo SEM_INHIBITOR_ON
        Total
Fission 234                         n_activations - n_fissions          computata dal master
Energy  4345                        inhibited_energy                    incrementata dall'inibitore
Waste   342                         ???                                 ???

        Total    Remaining
Time    87s      50s                                                    computata dal master

        Total    Used   Free
Energy  1234     1204   30                                              computata dal master
 */

#ifndef STATS_H
#define STATS_H

#define PAREN_OPEN  "["
#define PAREN_CLOSE "]"

#define STAT_ATOMS                          0
#define STAT_WASTES                         1
#define STAT_FISSIONS                       2
#define STAT_TIMESTAMP                      3
#define STAT_SIM_STATUS                     4
#define STAT_CURR_ENERGY                    5
#define STAT_USED_ENERGY                    6
#define STAT_ACTIVATIONS                    7
#define STAT_INHIBITOR_ON                   8
#define STAT_INHIBITED_ENERGY               9

#define COMPUTED_STAT_TOTAL_ATOMS           10
#define COMPUTED_STAT_TOTAL_ENERGY          11
#define COMPUTED_STAT_INHIBITED_FISSIONS    12

#define HEADER_FREE         0
#define HEADER_USED         1
#define HEADER_TOTAL        2
#define HEADER_GLOBAL       3
#define HEADER_LAST_SEC     4
#define HEADER_REMAINING    5
#define HEADER_INHIBITOR    6

const char *headers[] = {
    [HEADER_FREE]       = "Free",
    [HEADER_USED]       = "Used",
    [HEADER_TOTAL]      = "Total",
    [HEADER_GLOBAL]     = "Global",
    [HEADER_LAST_SEC]   = "LastSec",
    [HEADER_REMAINING]  = "Remaining",
    [HEADER_INHIBITOR]  = "Inhibitor"
};

#define ROW_TIME            0
#define ROW_ATOMS           1
#define ROW_WASTE           2
#define ROW_ENERGY          3
#define ROW_FISSION         4
#define ROW_ACTIVATIONS     5

const char *rows[] = {
    [ROW_TIME]          = "Time",
    [ROW_ATOMS]         = "Atoms",
    [ROW_WASTE]         = "Waste",
    [ROW_ENERGY]        = "Energy",
    [ROW_FISSION]       = "Fission",
    [ROW_ACTIVATIONS]   = "Att"
};

#endif