#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

#include "../libs/config.h"
#include "../libs/console.h"
#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"

struct Model *model;
struct IpcRes res;

void sigterm_handler() {
    // TODO signal masking to prevent other signals from interrupting this handler
    pid_t data = -1;
    if (write(res.fifo_fd, &data, sizeof(pid_t)) == -1) {
        errno_fail("NON ABBIAMO SCRITTO (alimentatore).\n", F_INFO);
    }
    DEBUG_BREAKPOINT;
    free_ipc();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    printf("Alimentatore: %d\n", getpid());

    init_ipc(&res, ALIMENTATORE);

    if (parse_long(argv[1], (long *) &res.shmid) == -1) {
        fail("Could not parse shmid (%s).\n", F_INFO, argv[1]);
    }

    attach_shmem();
    attach_model();

    open_fifo(O_WRONLY);

    char *argvc[3];
    char buf[INT_N_CHARS];
    prepare_argv(argvc, buf, "atomo", res.shmid);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        errno_fail("Could not set SIGTERM handler.\n", F_INFO);
    }

    while (1) {
        nano_sleep(STEP_ALIMENTAZIONE);

        for (int i = 0; i < N_NUOVI_ATOMI; i++) {
            switch (fork()) {
                case -1:
                    errno_fail("Could not fork.\n", F_INFO);
                    break;
                case 0:
                    execve(argvc[0], argvc, NULL);
                    errno_fail("Could not execute %s.\n", F_INFO, argvc[0]);
                    break;
                default:
                    break;
            }
        }
    }
}