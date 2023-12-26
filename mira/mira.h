#ifndef MIRA
#define MIRA

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

#define ITC_SIZE ((3 * sizeof(int) + 1) * sizeof(char))

#define DEBUG_BREAKPOINT printf("%s:%d\n", __FILE__, __LINE__)

#define ATOM        0
#define INHIBITOR   1
#define MASTER      2
#define INH_ON      3

int *attach_shmem(int shmid);
void detach_shmem();

pid_t fork_execve(char **argv);
void prargs(char *executable, char ***argv, char **buf, int vargs, size_t elemsize);
void frargs(char **argv, char *buf);

int parse_long(char *raw, long *dest);
int parse_int(char *raw, int *dest);

#endif