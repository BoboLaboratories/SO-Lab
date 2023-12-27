#include "model.h"
#include "../lifo/lifo.h"

void init_model(void *shmaddr) {
    extern enum Component component;
    extern struct Config *config;
    extern struct Stats *stats;

    config = (struct Config *) shmaddr;
    stats = (struct Stats *) shmaddr + sizeof(struct Config);

    if (component == ATOMO) {
        extern struct Lifo *lifo;
        lifo = (struct Lifo *) stats + sizeof(struct Stats);
    }
}
