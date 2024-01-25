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

#include "model.h"
#include "master.h"

struct PrintableStats {
    int inhibitor_off;
    enum Status status;
    struct Stats stats;
    unsigned long remaining_seconds;
};

#define STAT_ATOMS                          0
#define STAT_WASTES                         1
#define STAT_FISSIONS                       2
#define STAT_CURR_ENERGY                    3
#define STAT_USED_ENERGY                    4
#define STAT_ACTIVATIONS                    5
#define STAT_INHIBITOR_OFF                  6
#define STAT_INHIBITED_ENERGY               7
#define COMPUTED_STAT_TOTAL_ATOMS           8

#define COMPUTED_STAT_TOTAL_ENERGY          9
#define COMPUTED_STAT_REMAING_SECONDS       10

#define STAT_COUNT                          11


#define HEADER_FREE         (BLUE "Free" RESET)
#define HEADER_USED         (BLUE "Used" RESET)
#define HEADER_TOTAL        (BLUE "Total" RESET)
#define HEADER_GLOBAL       (BOLD_BLUE "Global" RESET)
#define HEADER_LAST_SEC     (BLUE "LastSec" RESET)
#define HEADER_REMAINING    (BLUE "Remaining" RESET)
#define HEADER_INHIBITOR    (BLUE "Inhibitor" RESET)

#define HEADER_MAX_LEN          9
#define HEADER_MAX_COLORED_LEN  (9 + 11)


#define ROW_TIME            (BLUE "    Time  " RESET)
#define ROW_ATOMS           (BLUE "   Atoms  " RESET)
#define ROW_WASTE           (BLUE "   Waste  " RESET)
#define ROW_ENERGY          (BLUE "  Energy  " RESET)
#define ROW_FISSION         (BLUE " Fission  " RESET)
#define ROW_ACTIVATIONS     (BLUE "     Att  " RESET)

#define ROW_MAX_LEN         10

void print_stats(struct PrintableStats prnt);

#endif