#include <stdlib.h>
#include <sys/shm.h>
#include <stdio.h>

#include "lifo.h"

int semid;

void init_lifo(int sem) {
    semid = sem;
}

pid_t pop() {

}

void push(pid_t pid) {

}