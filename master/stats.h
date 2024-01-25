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

void print_stats(struct PrintableStats prnt);

static void more_space();
static void achar(char c);
static void aspaces(unsigned long n);
static void along(unsigned long number);
static unsigned long astr(const char *str);
static void apad(const char *str, int is_number);
static void astrpad(const char *str);


#endif