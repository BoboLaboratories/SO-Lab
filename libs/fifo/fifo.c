#include "fifo.h"

int fifo_create(const char *pathname, mode_t flags) {
    int fifo_fd = mkfifo(pathname, flags);
    if (fifo_fd == -1) {
        errno_term("Could not create fifo (%s).\n", F_INFO, pathname);
    }
#if defined(DEBUG) || defined(D_FIFO)
    else {
        printf(D "Created fifo (%s).\n", pathname);
    }
#endif

    return fifo_fd;
}

int fifo_open(char *pathname, int flags) {
    int fifo_fd = open(pathname, flags);
    if (fifo_fd == -1) {
        errno_term("Could not open fifo (%s).\n", F_INFO, pathname);
    }
#if defined(DEBUG) || defined(D_FIFO)
    else {
        printf(D "Could not open fifo fifo (%s).\n", pathname);
    }
#endif

    return fifo_fd;
}

int fifo_close(int fifo_fd) {
    int ret = close(fifo_fd);
    if (ret == -1) {
        errno_term("Could not close fifo (%d).\n", F_INFO, fifo_fd);
    }
#if defined(DEBUG) || defined(D_FIFO)
    else {
        printf(D "Could not open fifo fifo (%d).\n", fifo_fd);
    }
#endif

    return ret;
}
