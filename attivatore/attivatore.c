#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include "../libs/ipc/ipc.h"
#include "../libs/util/util.h"
#include "../libs/config.h"
#include "../libs/console.h"

sig_atomic_t interrupted = 0;

void sigterm_handler() {
    // TODO signal masking to prevent other signals from interrupting this handler
    // TODO probably not needed as sig_atomic_t is atomic already
    interrupted = 1;
}

struct Model *model;
struct IpcRes *res;

int main(int argc, char *argv[]) {
    init_ipc(res, ATTIVATORE);

    if (parse_int(argv[1], &res->shmid) == -1) {
        fail("Could not parse shmid (%s).\n", F_INFO, argv[1]);
    }

    attach_shmem();
    attach_model();

    open_fifo(O_RDONLY);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        errno_fail("Could not set SIGTERM handler.\n", F_INFO);
    }

    pid_t pid;
    while (!interrupted) {
        nano_sleep(&interrupted, STEP_ATTIVATORE);
        int i = 0;
        while (!interrupted && i < 1) {
            // TODO read ritorna il numero di byte letti se interrotta a meta' da un segnale
            if (read(res->fifo_fd, &pid, sizeof(pid_t)) == -1) {

            } else {
                if (kill(inhib, SIGUSR1) == -1) {
                    pid_t atom = ...; // pid preso dalla FIFO
                    kill(atom, SIGUSR1);
                }

                // SIGUSR1 == Segnale di scissione
                semop(INH_SEM, 0, ...);
                pid_t dest = pid;
                // msg = {type: dest};
                if (shmem->inhibitor_pid != -1) {
                    dest = shmem->inhibitor_pid;
                    // msg = {type: dest, target_atom: pid};
                }

                //
                msq_put(msg);
                i++;
            }
        }
    }

    free_ipc();

    exit(EXIT_SUCCESS);

}