#include <stdio.h>
#include <errno.h>

#include "model.h"
#include "lib/sig.h"
#include "lib/fifo.h"

extern struct Model *model;
int fifo_fd;
int stop = 0;
int n_chunk = 0;

#define CHUNK 100

void add(int i) {
    if (write(fifo_fd, &i, sizeof(int)) == -1) {
        if (errno == EAGAIN) {
            ssize_t size = fcntl(fifo_fd, F_GETPIPE_SZ);
            print(D, "size before: %d\n", size / sizeof(int));
            // 1048576
            if (n_chunk < 3) {
                if (fcntl(fifo_fd, F_SETPIPE_SZ, size + 1) == -1) {
                    print(E, "Could not set fifo buffer.\n");
                } else {
                    size = fcntl(fifo_fd, F_GETPIPE_SZ);
                    print(D, "size after: %d\n\n", size / sizeof(int));
                    n_chunk++;
                }
            } else {
                stop = 1;
            }
        }
    }

//    if (fcntl(fifo_fd, F_SETPIPE_SZ, 70000 * sizeof(pid_t)) == -1) {
//        print(E, "Could not set fifo buffer.\n");
//    }

//    if (fcntl(fifo_fd, F_SETPIPE_SZ, 70000 * sizeof(pid_t)) == -1) {
//        print(E, "Could not set fifo buffer.\n");
//        exit(1);
//    }
}

int main(int argc, char *argv[]) {

    fifo_fd = mkfifo("tmp.txt", S_IWUSR | S_IRUSR);

    fifo_fd = open("tmp.txt", O_RDWR);

    int fifo_flags = fcntl(fifo_fd, F_GETFL, 0);
    fcntl(fifo_fd, F_SETFL, fifo_flags | O_NONBLOCK);

    int i = 0;
    while (!stop) {
        add(i++);
    }

    print(I, "Stopped %d\n", i);
    unlink("tmp.txt");
}

void cleanup() {

}

