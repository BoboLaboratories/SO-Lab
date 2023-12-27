#ifndef LIBS_IPC_H
#define LIBS_IPC_H

#include <sys/types.h>
#include "../lifo/lifo.h"

#define IPC_DIRECTORY ".ipc"
#define FIFO_PATHNAME IPC_DIRECTORY "/fifo"


//struct IpcRes {
//    enum Component component;
//    int shmid;
//    void *addr;
//    int fifo_fd;
//};




void init_ipc(struct IpcRes **res, enum Component component);
void attach_model();
void attach_shmem();
void free_ipc();

// fifo
void open_fifo(int flags);
void close_fifo();

// semaphores
void sem_sync(int semid);

#endif