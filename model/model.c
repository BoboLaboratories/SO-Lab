#include <stdlib.h>

#include "model.h"

#define OFFSET_CONFIG   0
#define OFFSET_STATS    (OFFSET_CONFIG + sizeof(struct Config))
#define OFFSET_IPC      (OFFSET_STATS + sizeof(struct Stats))
#define OFFSET_LIFO     (OFFSET_IPC + sizeof(struct Ipc))

struct Model *model = NULL;

extern void cleanup();
static void cleanup_model();
static void signal_handler(int signum);

void init() {
#ifdef DEBUG
    print(D, "Init\n");
    setbuf(stdout, NULL);
#endif

    model = malloc(sizeof(struct Model));
    model->res = malloc(sizeof(struct Resources));

    model->res->shmid = -1;
    model->res->shmaddr = (void *) -1;
#if defined(MASTER) || defined(ATTIVATORE) || defined(ALIMENTATORE)
    model->res->fifo_fd = -1;
#endif

    if (atexit(&cleanup_model) != 0) {
        print(E, "Could not register model cleanup function at exit.\n");
        cleanup();
        cleanup_model();
        exit(EXIT_FAILURE);
    }
    if (atexit(&cleanup) != 0) {
        print(E, "Could not register cleanup function at exit.\n");
        cleanup();
        exit(EXIT_FAILURE);
    }
}

void attach_model(void *shmaddr) {
    model->config = shmaddr + OFFSET_CONFIG;
    model->stats = shmaddr + OFFSET_STATS;
    model->ipc = shmaddr + OFFSET_IPC;

#if defined(MASTER) || defined(ATOMO) || defined(ATTIVATORE) || defined(INIBITORE)
    model->lifo = shmaddr + OFFSET_LIFO;
#endif
}

static void cleanup_model() {
    free(model->res);
    free(model);
}

//
//static int is_meaningful_signal(int signum) {
//    int meaningful = signum == SIGTERM;
//    for (int i = 0; !meaningful && MEANINGFUL_SIGNALS[i] != -1; i++) {
//        meaningful = signum == MEANINGFUL_SIGNALS[i];
//    }
//    return meaningful;
//}
//
//int running() {
//#if !defined(ATOMO)
//    sig = -1;
//#endif
//
//    if (MEANINGFUL_SIGNALS[0] != -1) {
//        // while no meaningful signal is received
//        while (!is_meaningful_signal(sig)) {
//            // wait for children processes to terminate
//            while (wait(NULL) != -1) {
//#if defined(MASTER) || defined(ALIMENTATORE)
//                // if a child atom died, check for its exit status so that
//                // other processes can perform their job accordingly
//                struct sembuf sops;
//                sem_buf(&sops, SEM_ALIMENTATORE, +1, 0);
//                if (sem_op(model->ipc->semid, &sops, 1) == -1)  {
//                    // TODO
//                }
//#endif
//            }
//
//            // if this process has no children
//            if (errno == ECHILD) {
//                // wait until a signal is received
//                // when pause is interrupted by a signal,
//                pause();
//                // break the inner loop so that meaningful
//                // signals are checked by the outer one
//                break;
//            }
//        }
//    }
//
//    return sig != SIGTERM;
//}