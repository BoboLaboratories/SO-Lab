#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>

#include "../lib/lifo/lifo.h"
#include "../lib/sem/sem.h"
#include "../lib/shmem/shmem.h"

#define MASTER

sig_atomic_t interrupted;
struct Model *model;

int main(int argc, char *argv[]) {
    int init[] = { 1000 + 1, 12 };
    int semid = mksem(IPC_PRIVATE, 2, 0777, init);
    printf("semid=%d\n", semid);

    printf("0=%d\n", semctl(semid, 0, GETVAL));
    printf("1=%d\n", semctl(semid, 1, GETVAL));

    rmsem(semid);



//    int semid = semget(IPC_PRIVATE, 1, S_IWUSR | S_IRUSR);
//
//    union semun se;
//    se.val = SHRT_MAX + 1;
//    int res = semctl(semid, 0, SETVAL, se);
//    printf("res: %d\n", res);

//
//    struct Lifo lifo;
//    mklifo(&lifo, 2, sizeof(int), semid, 0);
//
//    for (int i = 0; i < 5; i++) {
//        lifo_push(&lifo, &i);
//        printf("Pushing: %d, length: %d\n", i, lifo.length);
//    }
//
//    for (int i = 0; i < 5; i++) {
//        int n = 0;
//        lifo_pop(&lifo, &n);
//        printf("Popping: %d, length: %d\n", n, lifo.length);
//    }
//
//
//    int n = 12312;
//    lifo_push(&lifo, &n);
//    printf("push: %d, length: %d\n", n, lifo.length);
//    n = 0;
//    lifo_pop(&lifo, &n);
//    printf("pop: %d, length: %d\n", n, lifo.length);
//
//    rmlifo(&lifo);
}

void cleanup() {

}