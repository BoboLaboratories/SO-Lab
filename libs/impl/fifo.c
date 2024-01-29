#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "lib/fifo.h"
#include "lib/util.h"
#include "lib/console.h"

int fifo_create(const char *pathname, mode_t flags) {
    int fd = mkfifo(pathname, flags);

    if (fd == -1) {
        print(E, "Could not create fifo (%s).\n", pathname);
    } else {
        // remember fifo file so that it's
        // automatically unlinked on exit
        addtmpfile(pathname);
    }

    return fd;
}

int fifo_open(const char *pathname, int flags) {
    int fd = open(pathname, flags);

    if (fd == -1) {
        print(E, "Could not open fifo (%s).\n", pathname);
    } else {
        // set non-blocking fifo operations
        int fifo_flags;
        if ((fifo_flags = fcntl(fd, F_GETFL, 0)) == -1 || fcntl(fd, F_SETFL, fifo_flags | O_NONBLOCK) == -1) {
            print(E, "Could not set non-blocking fifo operations.\n");
        }
    }

    return fd;
}

static int fifo_add_(int fd, void *data, ssize_t size) {
    int ret = 0;

    if (write(fd, data, size) != size) {
        if (errno == EAGAIN) {
            // if fifo run out of space (due to very fast alimentatore or very slow attivatore)
            // request more buffer space from the operating system, then retry insertion
            //      note that, according to man, fcntl(fd, F_SETPIPE_SZ, size)
            //      rounds the buffer size to the next buffer page size
            //      that's why fsize + 1 suffices
            ssize_t fsize;
            if ((fsize = fcntl(fd, F_GETPIPE_SZ)) == -1 || fcntl(fd, F_SETPIPE_SZ, fsize + 1) == -1) {
                print(E, "Could not request more fifo buffer space.\n");
            }
        } else {
            print(E, "Could not insert data info fifo.\n");
        }
        ret = -1;
    }

    return ret;
}

int fifo_add(int fd, void *data, ssize_t size) {
    int ret;

    // if first try fails due to fifo buffer being full
    // try just one more time then return error
    if ((ret = fifo_add_(fd, data, size)) == -1) {
        ret = fifo_add_(fd, data, size);
    }

    return ret;
}

int fifo_remove(int fd, void *data, ssize_t size) {
    // since read is non-blocking for this fifo
    // implementation, just return -1 if read fails
    ssize_t bytes = read(fd, data, size);
    return bytes == size ? 0 : -1;
}

int fifo_close(int fd) {
    int ret = close(fd);

    if (ret == -1) {
        print(E, "Could not close fifo (%d).\n", fd);
    }

    return ret;
}