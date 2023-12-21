#include <fcntl.h>
#include <unistd.h>

#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"

struct Model *model;

int main(int argc, char *argv[]) {
    struct IpcRes res;

    init_ipc(&res, ATOMO);

    if (parse_long(argv[1], (long *) &res.shmid) == -1) {
        fail("Could not parse shmid (%s).\n", F_INFO, argv[1]);
    }

    attach_shmem();
    attach_model();

    open_fifo(O_WRONLY);

    pid_t pid = getpid();
    if (write(res.fifo_fd, &pid, sizeof(pid_t)) == -1) {
        errno_fail("NON ABBIAMO SCRITTO.\n", F_INFO);
    }

    free_ipc();

}