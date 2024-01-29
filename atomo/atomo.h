#ifndef ATOMO_H
#define ATOMO_H

#define EXIT_NATURAL   0
#define EXIT_INHIBITED 1

static void waste(int status);
static void split(int *atomic_number, int *child_atomic_number);
static void end_activation_cycle();
static void terminate();

#endif