#ifndef LIBS_FIFO_H
#define LIBS_FIFO_H

#include <sys/types.h>

int fifo_create(const char *pathname, mode_t mode);
int fifo_open(const char *pathname, int flags);
int fifo_add(int fd, void *data, ssize_t size);
int fifo_remove(int fifo_fd, void *data, ssize_t size);
int fifo_close(int fd);

#endif