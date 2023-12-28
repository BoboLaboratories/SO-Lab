#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "cleanup.h"
#include "../libs/console.h"

static void rmtmpfile();

const char *mktmpfile() {
    if (open(TMP_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR) == -1) {
        fprintf(stderr, E "Could not create temporary ipc file.\n");
        exit(EXIT_FAILURE);
    } else {
#ifdef DEBUG
        printf(D "Created temporary ipc file (%s).\n", tmp_file);
#endif
        if (atexit(&rmtmpfile) != 0) {
            fprintf(stderr, W "Could not register temporary ipc file removal at exit.\n");
        }
    }

    return TMP_FILE;
}

static void rmtmpfile() {
    if (unlink(TMP_FILE) == -1) {
        fprintf(stderr, E "Could not remove temporary ipc file (%s).\n", tmp_file);
    }
#ifdef DEBUG
    else {
        printf(D "Deleted temporary ipc file (%s).\n", tmp_file);
    }
#endif
}