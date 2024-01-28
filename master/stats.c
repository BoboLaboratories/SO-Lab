#include <string.h>
#include <malloc.h>

#include "stats.h"
#include "lib/util.h"
#include "lib/console.h"

// sim indexes in data array
#define STAT_ATOMS                      0
#define STAT_WASTES                     1
#define STAT_FISSIONS                   2
#define STAT_CURR_ENERGY                3
#define STAT_USED_ENERGY                4
#define STAT_ACTIVATIONS                5
#define STAT_INHIBITOR_OFF              6
#define STAT_INHIBITED_ATOMS            7
#define STAT_INHIBITED_ENERGY           8
#define COMPUTED_STAT_TOTAL_ATOMS       9
#define COMPUTED_STAT_TOTAL_ENERGY      10
#define COMPUTED_STAT_REMAING_SECONDS   11
#define STAT_COUNT                      12

// sim headers
#define HEADER_MAX_LEN         9
#define HEADER_COLOR           GREEN
#define HEADER_BOLD_COLOR      BOLD_GREEN
#define HEADER_COLORED_MAX_LEN (HEADER_MAX_LEN + 11)
#define HEADER_FREE            (HEADER_COLOR      "Free"      RESET)
#define HEADER_USED            (HEADER_COLOR      "Used"      RESET)
#define HEADER_TOTAL           (HEADER_COLOR      "Total"     RESET)
#define HEADER_GLOBAL          (HEADER_BOLD_COLOR "Global"    RESET)
#define HEADER_LAST_SEC        (HEADER_COLOR      "LastSec"   RESET)
#define HEADER_INHIBITOR       (HEADER_BOLD_COLOR "Inhibitor" RESET)

// sim rows
#define ROW_MAX_LEN     10
#define ROW_COLOR       BLUE
#define ROW_ATOMS       (ROW_COLOR "    Atoms  " RESET)
#define ROW_WASTE       (ROW_COLOR "   Wastes  " RESET)
#define ROW_ENERGY      (ROW_COLOR "   Energy  " RESET)
#define ROW_FISSION     (ROW_COLOR " Fissions  " RESET)
#define ROW_ACTIVATIONS (ROW_COLOR "     Acts  " RESET)

static char *printable_status[] = {
        [STARTING]   = GREEN   "STARTING"   RESET,
        [RUNNING]    = GREEN   "RUNNING"    RESET,
        [TIMEOUT]    = RED     "TIMEOUT"    RESET,
        [EXPLODE]    = RED     "EXPLODE"    RESET,
        [BLACKOUT]   = RED     "BLACKOUT"   RESET,
        [MELTDOWN]   = RED     "MELTDOWN"   RESET,
        [TERMINATED] = YELLOW  "TERMINATED" RESET
};

static char *printable_inhibitor[] = {
        [0] = GREEN "ON"  RESET,
        [1] = RED   "OFF" RESET
};


// the number of characters to add every time the buffer is full
#define INCREMENT 100

// max length, in character, occupied by a single cell
static unsigned long max_cell_len;

// buffer for the string that's being built
static char *buf;

// pointer to the next free character
static char *ptr;

// pointer to the end of the buffer
static char *endptr;

// buffer for writing numerical values
static char *numbuf;

// length, in character, of the buffer
static int buf_len;

// how many time sim have been printed
static int prnt_count = 0;

// sim from the previous iteration
static unsigned long *prev = NULL;

void print_stats(struct SimulationStats prnt) {
    extern struct Model *model;

    max_cell_len = HEADER_MAX_LEN;  // a cell must be wide enough to print headers in a table
    buf = numbuf = NULL;
    ptr = endptr = NULL;
    buf_len = 0;

    if (prev == NULL) {
        // if this is the first run, initialize prev
        prev = calloc(STAT_COUNT, sizeof(unsigned long));
        memset(prev, 0, STAT_COUNT * sizeof(unsigned long));
    }


    // produce computed sim
    unsigned long data[] = {
            [STAT_ATOMS] = prnt.stats.n_atoms,
            [STAT_WASTES] = prnt.stats.n_wastes,
            [STAT_FISSIONS] = prnt.stats.n_fissions,
            [STAT_CURR_ENERGY] = prnt.stats.curr_energy,
            [STAT_USED_ENERGY] = prnt.stats.used_energy,
            [STAT_ACTIVATIONS] = prnt.stats.n_activations,
            [STAT_INHIBITOR_OFF] = prnt.inhibitor_off,
            [STAT_INHIBITED_ATOMS] = prnt.stats.inhibited_atoms,
            [STAT_INHIBITED_ENERGY] = prnt.stats.inhibited_energy,
            [COMPUTED_STAT_TOTAL_ATOMS] = prnt.stats.n_atoms + prnt.stats.n_wastes,
            [COMPUTED_STAT_TOTAL_ENERGY] = prnt.stats.inhibited_energy + prnt.stats.curr_energy + prnt.stats.used_energy,
            [COMPUTED_STAT_REMAING_SECONDS] = prnt.remaining_seconds
    };

    // find the biggest long value that must be printed
    unsigned long max = 0;
    for (int i = 0; i < STAT_COUNT; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }

    // get the number of characters needed to print
    // such value and update max_cell_len if needed
    int len = snprintf(NULL, 0, "%lu", max) + 1;
    if ((unsigned long) len > max_cell_len) {
        max_cell_len = len;
    }

    // allocate a buffer sufficient for sprintf'ing such value
    numbuf = malloc(max_cell_len * sizeof(char));

    // line 1
    aspaces(ROW_MAX_LEN);
    astr(HEADER_GLOBAL);
    astr(" [");
    astr(printable_status[prnt.status]);
    astr(" | ");
    sprintf(numbuf, "%lds/", prnt.remaining_seconds);
    astr(numbuf);
    sprintf(numbuf, "%lds", SIM_DURATION);
    astr(numbuf);
    achar(']');
    achar('\n');

    // line 2
    aspaces(ROW_MAX_LEN);
    astrpad(HEADER_TOTAL);
    astrpad(HEADER_LAST_SEC);
    achar('\n');

    // line 3
    astr(ROW_ATOMS);

    along(data[STAT_ATOMS]);
    along(max(0, (signed) (data[STAT_ATOMS] - prev[STAT_ATOMS])));
    achar('\n');

    // line 4
    astr(ROW_WASTE);
    along(data[STAT_WASTES]);
    along(data[STAT_WASTES] - prev[STAT_WASTES]);
    achar('\n');

    // line 5
    astr(ROW_FISSION);
    along(data[STAT_FISSIONS]);
    along(data[STAT_FISSIONS] - prev[STAT_FISSIONS]);
    achar('\n');

    // line 6
    astr(ROW_ACTIVATIONS);
    along(data[STAT_ACTIVATIONS]);
    along(data[STAT_ACTIVATIONS] - prev[STAT_ACTIVATIONS]);
    achar('\n');

    // line 7
    achar('\n');

    // line 8
    aspaces(ROW_MAX_LEN);
    astr(HEADER_INHIBITOR);
    astr(" [");
    astr(printable_inhibitor[prnt.inhibitor_off]);
    achar(']');
    achar('\n');

    // line 9
    aspaces(ROW_MAX_LEN);
    astrpad(HEADER_TOTAL);
    astrpad(HEADER_LAST_SEC);
    achar('\n');

    // line 10
    astr(ROW_WASTE);
    along(data[STAT_INHIBITED_ATOMS]);
    along(data[STAT_INHIBITED_ATOMS] - prev[STAT_INHIBITED_ATOMS]);
    achar('\n');

    // line 11
    astr(ROW_ENERGY);
    along(data[STAT_INHIBITED_ENERGY]);
    along(data[STAT_INHIBITED_ENERGY] - prev[STAT_INHIBITED_ENERGY]);
    achar('\n');

    // line 12
    achar('\n');

    // line 13
    aspaces(ROW_MAX_LEN);
    astrpad(HEADER_TOTAL);
    astrpad(HEADER_LAST_SEC);
    astrpad(HEADER_USED);
    astrpad(HEADER_FREE);
    achar('\n');

    // line 14
    astr(ROW_ENERGY);
    along(data[COMPUTED_STAT_TOTAL_ENERGY]);
    along(data[COMPUTED_STAT_TOTAL_ENERGY] - prev[COMPUTED_STAT_TOTAL_ENERGY]);
    along(data[STAT_USED_ENERGY]);
    along(data[STAT_CURR_ENERGY]);
    achar('\n');

    // terminate string
    achar('\0');
    printf("\n%s", buf);

    if (prnt.status == RUNNING) {
        memcpy(prev, &data, STAT_COUNT * sizeof(unsigned long));
        printf("\n--------------------------------------------\n");
    } else {
        printf("\n");
        free(prev);
    }

    prnt_count++;

    free(numbuf);
    free(buf);
}

// insert a single char and request more space if needed
static void achar(char c) {
    if (ptr == endptr) {
        more_space();
    }
    *ptr = c;
    ptr++;
}

// insert n amount of spaces
static void aspaces(unsigned long n) {
    while (n > 0) {
        achar(' ');
        n--;
    }
}

// append a padded long
static void along(unsigned long number) {
    sprintf(numbuf, "%lu", number);
    apad(numbuf, 1);
}

// append a string without padding
static unsigned long astr(const char *str) {
    unsigned long i = 0;
    while (*str != '\0') {
        achar(*str);
        str++;
        i++;
    }
    return i;
}

// append a padded string or a sprintf'ed numerical value
static void apad(const char *str, int is_number) {
    aspaces((is_number ? HEADER_MAX_LEN : HEADER_COLORED_MAX_LEN) - astr(str));
}

// append a padded string
static void astrpad(const char *str) {
    apad(str, 0);
}

// add more buffer space
static void more_space() {
    buf_len += INCREMENT;
    buf = reallocarray(buf, buf_len, sizeof(char));
    endptr = buf + (buf_len * sizeof(char));
    if (ptr == NULL) {
        ptr = buf;
    }
}