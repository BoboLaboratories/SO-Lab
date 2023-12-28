#ifndef CONSOLE_H
#define CONSOLE_H

#define I   0,   NULL,      -1
#define E   1, __FILE__, __LINE__
#define D   2, __FILE__, __LINE__
#define W   3, __FILE__, __LINE__

void print(int mode, char *file, int line, char *format, ...);

// TODO remove
#define DEBUG_BREAKPOINT printf("%s:%d\n", __FILE__, __LINE__)

#endif
