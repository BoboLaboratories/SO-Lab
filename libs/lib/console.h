#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>

// reset color
#define RESET       "\e[0m"

// colors
#define RED         "\e[0;31m"
#define GREEN       "\e[0;32m"
#define BLUE        "\e[0;34m"
#define YELLOW      "\e[0;33m"

// bold colors
#define BOLD_RED    "\e[1;31m"
#define BOLD_GREEN  "\e[1;32m"
#define BOLD_BLUE   "\e[1;34m"
#define BOLD_YELLOW "\e[1;33m"

// output mode shorthand
#define I   0,   NULL,      -1
#define E   1, __FILE__, __LINE__
#define D   2, __FILE__, __LINE__
#define W   3, __FILE__, __LINE__

void print(int mode, char *file, int line, char *format, ...);

// TODO remove
#define DEBUG_BREAKPOINT printf("%d %s:%d\n", getpid(), __FILE__, __LINE__)

#endif
