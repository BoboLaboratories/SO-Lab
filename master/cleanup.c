#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "cleanup.h"
#include "../libs/ipc.h"
#include "../libs/console/console.h"

static void rmtmpfile();

const char *mktmpfile() {
    if (open(TMP_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR) == -1) {
        print(E, "Could not create temporary ipc file.\n");
        exit(EXIT_FAILURE);
    } else {
#ifdef DEBUG
        print(D, "Created temporary ipc file (%s).\n", TMP_FILE);
#endif
        if (atexit(&rmtmpfile) != 0) {
            print(W, "Could not register temporary ipc file removal at exit.\n");
        }
    }

    return TMP_FILE;
}

static void rmtmpfile() {
    if (unlink(TMP_FILE) == -1) {
        print(E, "Could not remove temporary ipc file (%s).\n", TMP_FILE);
    }
#ifdef DEBUG
    else {
        print(D, "Deleted temporary ipc file (%s).\n", TMP_FILE);
    }
#endif
}