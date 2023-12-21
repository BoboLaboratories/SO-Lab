#include <fcntl.h>
#include <malloc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "config.h"
#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"
#include "../libs/config.h"

void setup_ipc(struct IpcRes *res);
void setup_shmem(struct IpcRes *res);
void setup_fifo();

struct Model *model;

int main() {
    struct IpcRes res;

    setbuf(stdout, NULL); // TODO si vuole?
    setbuf(stderr, NULL); // TODO si vuole?

    setup_ipc(&res);
    attach_model();
    load_config();

    printf("%ld\n", N_ATOMI_INIT);

    open_fifo(O_RDWR);

    char *buf = NULL;
    char **argv = prargs(buf, "alimentatore", "%d", res.shmid);
    if (fork_execve(argv) == -1) {
        printf("PROBLEMA.\n");
    }

    free(buf);
    free(argv);

    pid_t pid;
    ssize_t result;
    while ((result = read(res.fifo_fd, &pid, sizeof(pid_t))) != -1 && pid != -1) {
        printf("Pid: %d\n", pid);
    }
    if (result == -1) {
        errno_fail("Failed to read.", F_INFO);
    }

    free_ipc();
}

void setup_ipc(struct IpcRes *res) {
    if (mkdir(IPC_DIRECTORY, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH) == -1) {
        errno_fail("Could not create IPC directory.\n", F_INFO);
    }

    init_ipc(res, MASTER);
    setup_shmem(res);

    setup_fifo();
}

void setup_shmem(struct IpcRes *res) {
    size_t size = sizeof(struct Config) + sizeof(struct Stats);
    res->shmid = shmget(IPC_PRIVATE, size, 0666 | IPC_CREAT);
    if (res->shmid == -1) {
        errno_fail("Could not create shared memory.\n", F_INFO);
    }

    #ifdef DEBUG
        printf(D "Obtained shared memory from OS (shmid: %d)\n", res->shmid);
    #endif

    attach_shmem();
}

void setup_fifo() {
    if (mkfifo(FIFO_PATHNAME, S_IWUSR | S_IRUSR) == -1) {
        errno_fail("Could not create fifo.\n", F_INFO);
    }
}