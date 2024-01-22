#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "model.h"
#include "lib/sem.h"
#include "lib/sig.h"
#include "lib/fifo.h"
#include "lib/util.h"
#include "lib/shmem.h"

extern struct Model *model;
extern sig_atomic_t sig;

int running();

static int select_atom(pid_t *atom);
static int acquire(int sem_num);
static int release(int sem_num);

static void default_handler(int signum) {
    sig = signum;
}

int main(int argc, char *argv[]) {
#ifdef D_PID
    print(D, "Attivatore: %d\n", getpid());
#endif

    if (argc != 2) {
        print(E, "Usage: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init();


    // =========================================
    //               Mask setup
    // =========================================
    sigset_t mask;
    sigset_t critical;
    sig_setup(&mask, &critical, SIGACTV, SIGWAST, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sig_handle(&default_handler, SIGALRM, SIGTERM);


    // =========================================
    //          Setup shared memory
    // =========================================
    if (parse_int(argv[1], &model->res->shmid) == -1) {
        print(E, "Could not parse shmid (%s).\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if ((model->res->shmaddr = shmem_attach(model->res->shmid)) == (void *) -1) {
        exit(EXIT_FAILURE);
    }

    attach_model(model->res->shmaddr);


    // =========================================
    //              Setup fifo
    // =========================================
    if ((model->res->fifo_fd = fifo_open(FIFO, O_RDONLY)) == -1) {
        exit(EXIT_FAILURE);
    }

//    activate non-blocking fifo read
//    int fifo_flags = fcntl(model->res->fifo_fd, F_GETFL, 0);
//    fcntl(model->res->fifo_fd, F_SETFL, fifo_flags | O_NONBLOCK);

    // Sem Sync
    sem_sync(model->ipc->semid, SEM_SYNC);


    timer_t timer = timer_start(STEP_ATTIVATORE);
    while (running()) {
        if (acquire(SEM_ATTIVATORE) == -1 && errno == EINTR) {
            continue;
        }

        pid_t atom = -1;
        if (select_atom(&atom) == -1 && errno == EINTR) {
            print(E, "Error while selecting new atom.\n");
            release(SEM_ATTIVATORE);
            continue;
        }

        mask(SIGALRM, SIGTERM); // TODO SIGTERM?
        acquire(SEM_MASTER);
        if (kill(atom, SIGACTV) == -1) {
            print(E, "Could not activate atom %d.\n", atom);
        } else {
            model->stats->n_activations++;
        }
        // we do not release SEM_MASTER given
        // the activation transaction has begun
        unmask(SIGALRM, SIGTERM);
    }
    timer_delete(timer);

    exit(EXIT_SUCCESS);
}

static struct sembuf sops;

static int acquire(int sem_num) {
    sem_buf(&sops, sem_num, -1, 0);
    return sem_op(model->ipc->semid, &sops, 1);
}

static int release(int sem_num) {
    mask(SIGALRM);
    sem_buf(&sops, sem_num, +1, 0);
    int ret = sem_op(model->ipc->semid, &sops, 1);
    unmask(SIGALRM);
    return ret;
}

static int select_atom(pid_t *atom) {
    int ret = -1;

    // first try to remove an atom from the lifo (recently activated atoms)
    mask(SIGALRM);
    ret = lifo_pop(model->lifo, atom);
    unmask(SIGALRM);

    // if no atoms were present, try removing from lifo (waiting atoms)
    if (ret == -1) {
        ret = fifo_remove(model->res->fifo_fd, atom, sizeof(pid_t));
    }

    return ret;
}

void cleanup() {
    if (model != NULL) {
        if (model->res->fifo_fd != -1) {
            fifo_close(model->res->fifo_fd);
        }
        if (model->res->shmaddr != (void *) -1) {
            shmem_detach(model->res->shmaddr);
        }
    }
}

int running() {
    while (!sig_is_handled(sig)) {
        pause();
    }

    return sig_reset(sig != SIGTERM);
}