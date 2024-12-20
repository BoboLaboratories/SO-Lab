#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define SEM_SYNC            0
#define SEM_ATOM            1
#define SEM_LIFO            2
#define SEM_MASTER          3
#define SEM_ALIMENTATORE    4
#define SEM_ATTIVATORE      5
#define SEM_INIBITORE       6
#define SEM_INIBITORE_OFF   7

#define SEM_COUNT           8

#define SIGMELT SIGUSR1     // when received by master it means meltdown
#define SIGACTV SIGUSR1     // when received by atom it means it should perform a fission
#define SIGWAST SIGUSR2     // when received by atom it means it should become waste

#define FIFO        ".so_fifo"
#define FTOK_PROJ   'x'

struct Ipc {
    int semid;
    pid_t master_pid;
    pid_t alimentatore_pid;
};

#endif