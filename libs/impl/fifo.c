#include <errno.h>

#include "lib/fifo.h"

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

    int fifo_flags = fcntl(fifo_fd, F_GETFL, 0);
    if (fcntl(fifo_fd, F_SETFL, fifo_flags | O_NONBLOCK) == -1) {
        print(E, "Could not set non-blocking fifo operations.\n");
        // TODO
    }

#if defined(DEBUG) || defined(D_FIFO)
    else {
        print(D, "Opened fifo (%s, fd=%d).\n", pathname, fifo_fd);
    }
#endif

    return fifo_fd;
}

int fifo_add(int fifo_fd, void *data, size_t size) {
    int ret = 0;

    if (write(fifo_fd, data, size) == -1) {
        if (errno == EAGAIN) {
            ssize_t fsize = fcntl(fifo_fd, F_GETPIPE_SZ);
            print(D, "size before: %d\n", fsize / size);
            if (fcntl(fifo_fd, F_SETPIPE_SZ, fsize + 1) == -1) {
                print(E, "Could not set fifo buffer.\n");
                return -1;
            } else {
                fsize = fcntl(fifo_fd, F_GETPIPE_SZ);
                print(D, "size after: %d\n\n", fsize / size);
                return fifo_add(fifo_fd, data, size);
            }
        }
    }

    return ret;
}

int fifo_remove(int fifo_fd, void *data, size_t size) {
    return (int) read(fifo_fd, data, size);
}

int fifo_close(int fifo_fd) {
    int ret = close(fifo_fd);
    if (ret == -1) {
        print(E, "Could not close fifo (%d).\n", fifo_fd);
    }
#if defined(DEBUG) || defined(D_FIFO)
    else {
        print(D, "Closed fifo (fd=%d).\n", fifo_fd);
    }
#endif

    return ret;
}
