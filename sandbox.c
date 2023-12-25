#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>

union semun {
    // value for SETVAL
    int val;
    // buffer for IPC_STAT, IPC_SET
    struct semid_ds *buf;
    // array for GETALL, SETALL
    unsigned short *array;
    // Linux specific part
#if defined(__linux__)
    // buffer for IPC_INFO
    struct seminfo *__buf;
#endif
};

#define ATOM        0
#define INHIBITOR   1
#define MASTER      2

sig_atomic_t interrupted = 0;

void sigterm_handler() {
    interrupted = 1;
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigterm_handler;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        printf("Could not set SIGTERM handler.\n");
        exit(EXIT_FAILURE);
    }

    int semid = semget(IPC_PRIVATE, 3, S_IWUSR | S_IRUSR);
    if (semid == -1) {
        printf("Could not get semaphore.\n");
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(IPC_PRIVATE, sizeof(int), 0666 | IPC_CREAT);
    if (shmid == 1) {
        printf("Could not get shared memory.\n");
        exit(EXIT_FAILURE);
    }

    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *) -1) {
        printf("Could not attach shared memory.\n");
        exit(EXIT_FAILURE);
    }

    int *val = (int *) addr;
    *val = 0;

    union semun se;
    se.val = 1;
    semctl(semid, ATOM, SETVAL, se);
    se.val = 0;
    semctl(semid, INHIBITOR, SETVAL, se);
    se.val = 1;
    semctl(semid, MASTER, SETVAL, se);

//    struct sembuf tmp;
//    tmp.sem_flg = IPC_NOWAIT;
//    tmp.sem_num = ATOM;
//    tmp.sem_op = 0;
//    if (semop(semid, &tmp, 1) == -1) {
//        if (errno == EAGAIN) {
//            printf("EAG\n");
//        } else {
//            printf("BOH\n");
//        }
//    } else {
//        printf(":D\n");
//    }
//
//    if (errno != EINTR) {
//        return 1;
//    }


    for (int i = 0; i < 100; i ++) {
        switch (fork()) {
            case -1:
                printf("Error forking atom %d.\n", getpid());
                interrupted = 1;
                break;
            case 0:
                while (!interrupted) {
                    struct sembuf sops[2];
                    memset(&sops, 0, sizeof(sops));

                    sops[0].sem_num = ATOM;
                    sops[0].sem_op = -1;

                    sops[1].sem_num = MASTER;
                    sops[1].sem_op = -1;

                    if (semop(semid, sops, 2) == -1) {
                        printf("Atom: start %d.\n", getpid());
                        interrupted = 1;
                        break;
                    }

                    *val = *val + 1;
//                    printf("A");

                    sops[0].sem_num = INHIBITOR;
                    sops[0].sem_op = 1;

                    if (semop(semid, sops, 1) == -1) {
                        printf("Atom: end %d.\n", getpid());
                        interrupted = 1;
                    }
                }
                return EXIT_SUCCESS;
        }
    }

    switch (fork()) {
        case -1:
            printf("Error forking inhibitor.\n");
            interrupted = 1;
            break;
        case 0:
            while (!interrupted) {
                struct sembuf sops[2];
                memset(&sops, 0, sizeof(sops));

                sops[0].sem_num = INHIBITOR;
                sops[0].sem_op = -1;

                if (semop(semid, sops, 1) == -1) {
                    printf("Inhibitor: start.\n");
                    interrupted = 1;
                    break;
                }

                *val = *val - 1;
//                printf("I");

                sops[0].sem_num = MASTER;
                sops[0].sem_op = 1;

                if (semop(semid, sops, 1) == -1) {
                    printf("Inhibitor: master wake up.\n");
                    interrupted = 1;
                }

                sops[0].sem_num = ATOM;
                sops[0].sem_op = 1;

                if (semop(semid, sops, 1) == -1) {
                    // TODO Boh forse non vale neanche come errore
                    printf("Inhibitor: wake up atom.\n");
                }
            }
            return EXIT_FAILURE;
    }

    while (!interrupted) {
        struct sembuf sops[2];
        memset(&sops, 0, sizeof(sops));

        sops[0].sem_num = MASTER;
        sops[0].sem_op = -1;

        if (semop(semid, sops, 1) == -1) {
            printf("Master: start.\n");
            interrupted = 1;
            break;
        }

//        if (*val != 0) {
            printf("val=%d\n", *val);
//        }

        sops[0].sem_num = MASTER;
        sops[0].sem_op = 1;

        if (semop(semid, sops, 1) == -1) {
            printf("Master: end %d %s.\n", errno, strerror(errno));
            interrupted = 1;
        }

        sleep(1);
    }

    while (wait(NULL) != -1)
        ;

    printf("Atom: %d\n", semctl(semid, ATOM, GETVAL));
    printf("Inhibitor: %d\n", semctl(semid, INHIBITOR, GETVAL));
    printf("Master: %d\n", semctl(semid, MASTER, GETVAL));

    semctl(semid, 0, IPC_RMID);
}