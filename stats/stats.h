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

#define STAT_COUNT                          13


#define HEADER_FREE         "Free"
#define HEADER_USED         "Used"
#define HEADER_TOTAL        "Total"
#define HEADER_GLOBAL       "Global"
#define HEADER_LAST_SEC     "LastSec"
#define HEADER_REMAINING    "Remaining"
#define HEADER_INHIBITOR    "Inhibitor"

#define HEADER_MAX_LEN      9


#define ROW_TIME            "    Time "
#define ROW_ATOMS           "   Atoms "
#define ROW_WASTE           "   Waste "
#define ROW_ENERGY          "  Energy "
#define ROW_FISSION         " Fission "
#define ROW_ACTIVATIONS     "     Att "

#define ROW_MAX_LEN         9

void print_stats(const unsigned long *data);

#endif