#ifndef LIBS_FIFO_H
#define LIBS_FIFO_H

#include <fcntl.h>
#include <sys/stat.h>

#include "../util/util.h"
#include "../console/console.h"

int fifo_create(const char *pathname, mode_t mode);
int fifo_open(const char *path, int flags);
int fifo_close(int fifo_fd);

#endif