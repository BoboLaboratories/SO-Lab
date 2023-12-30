#ifndef LIBS_FIFO_H
#define LIBS_FIFO_H

#include <fcntl.h>
#include <sys/stat.h>

#include "../util/util.h"
#include "../console/console.h"

#define FIFO ".so_fifo"

int fifo_create(const char *pathname, mode_t mode);
int fifo_open(const char *pathname, int flags);
int fifo_close(int fifo_fd);

#endif