#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>

#include "libs/lifo/lifo.h"
#include "libs/shmem/shmem.h"
#include "libs/model/model.h"

void init_model(void *baseaddr) {
    extern enum Component component;
    extern struct Config *config;
    extern struct Stats *stats;

    config = (struct Config *) baseaddr;
    stats = (struct Stats *) baseaddr + sizeof(struct Config);

    if (component == ATOMO) {
        extern struct Lifo *lifo;
        lifo = (struct Lifo *) stats + sizeof(struct Stats);
    }
}


struct Lifo *lifo;

int main() {
    void *baseaddr; // attach

    lifo = (struct Lifo *) (baseaddr + SHM_LIFO_OFFSET);
}