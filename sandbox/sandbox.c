#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define MASTER

sig_atomic_t interrupted = 0;
struct Model *model;

int MEANINGFUL_SIGNALS[] = {-1};


//void signal_handler() {
//    printf("Ciao mamma sono in tv\n");
//    interrupted++;
//}

timer_t timerr(long nanos) {
    // 1s -> 1e9 (nano)
    // 1s -> 1e6 (micro)
    timer_t timerid;
    timer_create(CLOCK_REALTIME, NULL, &timerid);

    struct itimerspec spec;
    spec.it_value.tv_sec = nanos / (long) 1e9;
    spec.it_value.tv_nsec = nanos % (long) 1e9;
    spec.it_interval = spec.it_value;

    printf("%ld %ld\n", spec.it_value.tv_sec, spec.it_value.tv_nsec);
    timer_settime(CLOCK_REALTIME, 0, &spec, NULL);

    return timerid;
}


int main(int argc, char *argv[]) {

    timer_t timerid = timerr((long) 1e9);

    while (1) {
        pause();
        printf("(%d)", interrupted);
        if (interrupted == 10) {
            timer_delete(timerid);
        }
    }



    printf("Done.\n");
}

void cleanup() {

}

