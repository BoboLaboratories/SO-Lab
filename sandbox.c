#include <stdio.h>
#include <fcntl.h>

#include "libs/lifo/lifo.h"

int main() {
    printf("%lu\n", sizeof(struct Page *));

    // 64000 * 4 = 256000 byte = 250 kb
    // 64000 * 4 + 640 * 8 = 254 kb

    init_lifo(123);

    for (int i = 1; i <= 5; i++) {
        push(i);
    }

    for (int i = 1; i <= 5; i++) {
        pop();
    }
    printf("\n\n");
    push(5);
    printf("%d\n", pop());
    printf("%d\n", pop());
}