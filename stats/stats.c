#include <string.h>

#include "stats.h"

static int max_len = -1;





void update_max_len() {
    if (max_len == -1) {
        for (int i = 0; i < sizeof(headers) / sizeof(headers[0]); i++) {
            int len = strlen(headers[i]);
            if (len > max_len) {
                max_len = len;
            }
        }
    }

    long arr[2];
    arr[S_ATOMS] = 500;
    arr[S_WASTES] = 500;
    for (int i = 0; i < 2; i++) {}

}

int main(int argc, char *argv[]) {


}