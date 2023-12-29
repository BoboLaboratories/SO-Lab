#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>

#include "stats/stats.h"

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

#define INCREMENT 100

static char *buf = NULL;
static char *ptr = NULL;
static char *endptr = NULL;

static char *segbuf = NULL;
static unsigned long max_len = HEADER_MAX_LEN;
static int arrlen = 0;

void more_space() {
    arrlen += INCREMENT;
    buf = reallocarray(buf, arrlen, sizeof(char));
    if (ptr == NULL) {
        ptr = endptr = buf;
    }
    endptr += INCREMENT * sizeof(char);
}

void update_max_len(const unsigned long *numbers, int n) {
    if (numbers != NULL) {
        unsigned long max = 0;
        for (int i = 0; i < n; i++) {
            if (numbers[i] > max) {
                max = numbers[i];
            }
        }
        char tmp[500];
        sprintf(tmp, "%lu", max);
        max = strlen(tmp) + 1;
        if (max > max_len) {
            max_len = max;
        }
    }

    segbuf = malloc(max_len * sizeof(char));
    if (buf == NULL) {
        more_space();
    }
}



void achar(char c) {
    if (ptr == endptr) {
        more_space();
    }
    *ptr++ = c;
}

void aspaces(unsigned long n) {
    while (n > 0) {
        achar(' ');
        n--;
    }
}

unsigned long astr(const char *str) {
    unsigned long i = 0;
    while (*str != '\0') {
        achar(*str++);
        i++;
    }
    return i;
}

void astrpad(const char *str) {
    aspaces(max_len - astr(str));
}

void along(unsigned long number) {
    sprintf(segbuf, "%lu", number);
    astrpad(segbuf);
}

const char *done() {
    *ptr = '\0';
    return buf;
}

unsigned long arr[] = {
        3,
        12345670221123,
        54132,
        4532,
        6532,
        777,
        12
};

int main(int argc, char *argv[]) {
    update_max_len(arr, 7);

    aspaces(ROW_MAX_LEN);
    astr(HEADER_GLOBAL);
    achar(' ');
    achar('[');
    astr("meltdown");
    achar(']');
    achar('\n');

    aspaces(ROW_MAX_LEN);
    astrpad(HEADER_FREE);
    astrpad(HEADER_USED);
    achar('\n');

    astr(ROW_ATOMS);
    along(arr[1]);
    along(4532);
    achar('\n');
    astr(ROW_FISSION);
    along(53212);
    along(453);
    achar('\n');

    printf("%s", done());
    printf("\n");

    printf("%lu\n", LONG_MAX);
    printf("%d\n", snprintf(NULL, 0, "%lu", LONG_MAX));
}