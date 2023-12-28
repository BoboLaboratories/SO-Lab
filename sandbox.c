#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libs/util/util.h"

sig_atomic_t interrupted;

int main(int argc, char *argv[]) {
    mktmpfile("ciao", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (mkfifo("fifo", S_IRUSR | S_IWUSR) != -1) {
        addtmpfile("fifo");
    }
    sleep(10);
}