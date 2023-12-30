#include "fifo.h"

int fifo_create(const char *pathname, mode_t flags) {
    int fifo_fd = mkfifo(pathname, flags);
    if (fifo_fd == -1) {
        print(E, "Could not create fifo (%s).\n", pathname);
    } else {
        addtmpfile(pathname);
#if defined(DEBUG) || defined(D_FIFO)
        print(D, "Created fifo (%s, fd=%d).\n", pathname, fifo_fd);
#endif
    }

    return fifo_fd;
}

int fifo_open(const char *pathname, int flags) {
    int fifo_fd = open(pathname, flags);
    if (fifo_fd == -1) {
        print(E, "Could not open fifo (%s).\n", pathname);
    }
#if defined(DEBUG) || defined(D_FIFO)
    else {
        print(D, "Opened fifo (%s, fd=%d).\n", pathname, fifo_fd);
    }
#endif

    return fifo_fd;
}

int fifo_close(int fifo_fd) {
    int ret = close(fifo_fd);
    if (ret == -1) {
        print(E,"Could not close fifo (%d).\n", fifo_fd);
    }
#if defined(DEBUG) || defined(D_FIFO)
    else {
        print(D, "Closed fifo (fd=%d).\n", fifo_fd);
    }
#endif

    return ret;
}
