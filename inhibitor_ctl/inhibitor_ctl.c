#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/sem.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {

    }

    key_t key;
    if ((key = ftok(FIFO, FTOK_PROJ)) == -1) {
        print(E, "Could not generate ftok key.\n");
        exit(EXIT_FAILURE);
    };

}