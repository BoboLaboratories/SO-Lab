#ifndef MASTERLIB_H
#define MASTERLIB_H


// MASTER
struct OpenResources {
    int ipc_dir;
    int shmid;
    void *shmaddr;
    int fifo_fd;
    int lifo;
};

// ATOM
struct OpenResources {
    int shmid;
    void *shmaddr;
};


static struct OpenResources *res;

struct OpenResources *init_res() {
    res = malloc()
}





#endif
