#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

/*
  Common prefixes used to style different types of messages

  N.B. Spaces are set manually to ensure legibility and identical
       spacing regardless of tab size and the nature of stdout/stderr
       (terminal in the case of master, logfile in case of players/pawns)
*/
#define M "        "
#define I "INFO:   "
#define W "WARN:   "
#define E "ERROR:  "
#define D "DEBUG:  "


/*
  Macro used to output an informative error report
*/
#define ERRNO_PRINT fprintf(stderr, M "Errno %d: %s (%s:%d, pid:%5d)\n", errno, strerror(errno), __FILE__, __LINE__, getpid())
#define F_INFO __LINE__, __FILE__

#define DEBUG_BREAKPOINT printf("%s:%d\n", __FILE__, __LINE__);

#endif
