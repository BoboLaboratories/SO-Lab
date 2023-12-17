#include <sys/shm.h>

#include "../libs/console.h"
#include "../libs/ipc/ipc.h"

struct IpcRes res;

void setup_shmem(int id);

int main() {
    init_ipc(&res, MASTER);
    setup_shmem(CTL);
    setup_shmem(MAIN);
}

void setup_shmem(int id) {
    res.shmid[id] = shmget(IPC_PRIVATE, sizeof(struct Ctl), 0666 | IPC_CREAT);
    if (res.shmid[id] == -1) {
        errno_fail("Could not create shared memory.\n", F_INFO);
    }
    attach_shmem(id);
}