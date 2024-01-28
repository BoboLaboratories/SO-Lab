#ifndef STATS_H
#define STATS_H

#include "model.h"
#include "master.h"

void print_stats(struct SimulationStats prnt);

static void more_space();
static void achar(char c);
static void aspaces(long n);
static void along(long number);
static  long astr(const char *str);
static void apad(const char *str, int is_number);
static void astrpad(const char *str);


#endif