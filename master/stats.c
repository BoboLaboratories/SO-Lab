#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "stats.h"
#include "lib/console.h"

// the amount of characters to add every time the buffer is full
#define INCREMENT 100

// max length of the text that's going to occupy a single cell
static unsigned long max_cell_len;

// buffer for the string that's being built
static char *buf;

// pointer to the next free character
static char *ptr;

// pointer to the end of the buffer
static char *endptr;

// buffer for writing numerical values
static char *numbuf;

// length of the buffered (in characters)
static int buf_len;

static char *printable_status[] = {
        [STARTING]   = GREEN   "STARTING"   RESET,
        [RUNNING]    = GREEN   "RUNNING"    RESET,
        [TIMEOUT]    = RED     "TIMEOUT"    RESET,
        [EXPLODE]    = RED     "EXPLODE"    RESET,
        [BLACKOUT]   = RED     "BLACKOUT"   RESET,
        [MELTDOWN]   = RED     "MELTDOWN"   RESET,
        [TERMINATED] = YELLOW  "TERMINATED" RESET
};


static void more_space();
static void achar(char c);
static void aspaces(unsigned long n);
static void along(unsigned long number);
static unsigned long astr(const char *str);
static void apad(const char *str, int is_number);
static void astrpad(const char *str);

int count = 0;

void print_stats(struct PrintableStats prnt) {
    max_cell_len = HEADER_MAX_LEN;
    buf = numbuf = NULL;
    ptr = endptr = NULL;
    buf_len = 0;

    count++;

    // find the biggest long value that must be printed
    unsigned long data[] = {
            [STAT_ATOMS] = prnt.stats.n_atoms,
            [STAT_WASTES] = prnt.stats.n_wastes,
            [STAT_FISSIONS] = prnt.stats.n_fissions,
            [STAT_CURR_ENERGY] = prnt.stats.curr_energy,
            [STAT_USED_ENERGY] = prnt.stats.used_energy,
            [STAT_ACTIVATIONS] = prnt.stats.n_activations,
            [STAT_INHIBITOR_OFF] = prnt.inhibitor_off,
            [STAT_INHIBITED_ENERGY] = prnt.stats.inhibited_energy,
            [COMPUTED_STAT_TOTAL_ATOMS] = prnt.stats.n_atoms + prnt.stats.n_wastes,
            [COMPUTED_STAT_TOTAL_ENERGY] = prnt.stats.inhibited_energy + prnt.stats.curr_energy + prnt.stats.used_energy,
            [COMPUTED_STAT_REMAING_SECONDS] = prnt.remaining_seconds
    };

    unsigned long max = 0;
    for (int i = 0; i < STAT_COUNT; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }

    // get the number of characters needed to print such value
    int len = snprintf(NULL, 0, "%lu", max) + 1;
    if ((unsigned long) len > max_cell_len) {
        max_cell_len = len;
    }

    // allocate a buffer sufficinent for sprintf'ing such value
    numbuf = malloc(max_cell_len * sizeof(char));

    aspaces(ROW_MAX_LEN);
    astr(HEADER_GLOBAL);
    astr(" [");
    astr(printable_status[prnt.status]);
    achar(']');
    achar('\n');

    aspaces(ROW_MAX_LEN);
    astrpad(HEADER_TOTAL);
    astrpad(HEADER_LAST_SEC);
    achar('\n');

    astr(ROW_ATOMS);
    along(data[STAT_ATOMS]);
    along(0);
    achar('\n');
    astr(ROW_WASTE);
    along(data[STAT_WASTES]);
    along(0);
    achar('\n');
    astr(ROW_FISSION);
    along(data[STAT_FISSIONS]);
    along(0);
    achar('\n');
    astr(ROW_ACTIVATIONS);
    along(data[STAT_ACTIVATIONS]);
    along(0);
    achar('\n');

    achar('\n');

    // terminate string
    achar('\0');
    printf("%s\n\n", buf);

    // reset
    free(numbuf);
    free(buf);
}

static void achar(char c) {
    if (ptr == endptr) {
        more_space();
    }
    *ptr = c;
    ptr++;
}

static void aspaces(unsigned long n) {
    while (n > 0) {
        achar(' ');
        n--;
    }
}

static void along(unsigned long number) {
    sprintf(numbuf, "%lu", number);
    apad(numbuf, 1);
}

static unsigned long astr(const char *str) {
    unsigned long i = 0;
    while (*str != '\0') {
        achar(*str);
        str++;
        i++;
    }
    return i;
}

static void apad(const char *str, int is_number) {
    aspaces((is_number ? HEADER_MAX_LEN : HEADER_MAX_COLORED_LEN) - astr(str));
}

static void astrpad(const char *str) {
    apad(str, 0);
}

static void more_space() {
    buf_len += INCREMENT;
    if ((buf = reallocarray(buf, buf_len, sizeof(char))) == NULL) {
        print(E, "boh\n");
    }
    if (ptr == NULL) {
        ptr = buf;
    }
    endptr += INCREMENT * sizeof(char);
}