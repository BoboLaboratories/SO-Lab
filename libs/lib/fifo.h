#ifndef LIBS_FIFO_H
#define LIBS_FIFO_H

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"
#include "console.h"

int fifo_create(const char *pathname, mode_t mode);
int fifo_open(const char *pathname, int flags);
int fifo_add(int fifo_fd, void *data, size_t size);
int fifo_remove(int fifo_fd, void *data, size_t size);
int fifo_close(int fifo_fd);

#endif