#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"

struct Model *model;
struct IpcRes *res;

/*
    [0] = executable
    [1] = shmid
    [2] = atomic number
    [3] = semid, if semaphore synchronization is needed; NULL, otherwise
    [4] = NULL, if semaphore synchronization is needed; not present, otherwise
 */
int main(int argc, char *argv[]) {
    init_ipc(&res, ATOMO);

    if (parse_int(argv[1], &res->shmid) == -1) {
        fail("Could not parse shmid (%s).\n", F_INFO, argv[1]);
    }

    attach_shmem();
    attach_model();

    open_fifo(O_WRONLY);

    if (argv[3] != NULL) {
        // if present, argv[2] contains the sync semaphore id
        int semid;
        if (parse_int(argv[2], &semid) == -1) {
            errno_fail("Could not parse semid.\n", F_INFO);
        }

        sem_sync(semid);
    }

    pid_t pid = getpid();
    if (write(res->fifo_fd, &pid, sizeof(pid_t)) == -1) {
        errno_fail("NON ABBIAMO SCRITTO.\n", F_INFO);
    }

    free_ipc();

    exit(EXIT_SUCCESS);
}