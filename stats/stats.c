#include <string.h>
#include <malloc.h>

#include "stats.h"

// the amount of characters to add every time the buffer is full
#define INCREMENT 100

// max length of the text that's going to occupy a single cell
static unsigned long max_cell_len;

// buffer for the string that's being built
static char *buf;

// pointer to the next free character
static char *ptr;

// pointer to the end of the buffer
static char *endptr;

// buffer for writing numerical values
static char *numbuf;

// length of the buffered (in characters)
static int buf_len;


static void more_space();
static void achar(char c);
static void aspaces(unsigned long n);
static void along(unsigned long number);
static unsigned long astr(const char *str);
static void astrpad(const char *str);


void print_stats(const unsigned long *data) {
    max_cell_len = HEADER_MAX_LEN;
    ptr = endptr = NULL;
    buf_len = 0;

    // find the biggest long value that must be printed
    unsigned long max = 0;
    for (int i = 0; i < STAT_COUNT; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }

    // get the number of characters needed to print such value
    int len = snprintf(NULL, 0, "%lu", max) + 1;
    if ((unsigned long) len > max_cell_len) {
        max_cell_len = len;
    }

    // allocate a buffer sufficinent for sprintf'ing such value
    numbuf = malloc(max_cell_len * sizeof(char));

    // TODO make string

    aspaces(ROW_MAX_LEN);
    astr(HEADER_GLOBAL);
    achar(' ');
    achar('[');
    astr("meltdown");
    achar(']');
    achar('\n');

    aspaces(ROW_MAX_LEN);
    astrpad(HEADER_FREE);
    astrpad(HEADER_USED);
    achar('\n');

    astr(ROW_ATOMS);
    along(data[1]);
    along(4532);
    achar('\n');
    astr(ROW_FISSION);
    along(53212);
    along(453);
    achar('\n');

    // terminate string
    *ptr = '\0';
    printf("%s", buf);

    // reset
    free(numbuf);
    free(buf);
}

void achar(char c) {
    if (ptr == endptr) {
        more_space();
    }
    *ptr++ = c;
}

void aspaces(unsigned long n) {
    while (n > 0) {
        achar(' ');
        n--;
    }
}

void along(unsigned long number) {
    sprintf(numbuf, "%lu", number);
    astrpad(numbuf);
}


unsigned long astr(const char *str) {
    unsigned long i = 0;
    while (*str != '\0') {
        achar(*str++);
        i++;
    }
    return i;
}

void astrpad(const char *str) {
    aspaces(max_cell_len - astr(str));
}

static void more_space() {
    buf_len += INCREMENT;
    buf = reallocarray(buf, buf_len, sizeof(char));
    if (ptr == NULL) {
        ptr = endptr = buf;
    }
    endptr += INCREMENT * sizeof(char);
}

int main() {
    unsigned long arr[] = {
        3,
        15652334512334,
        3,
        4532,
        6532,
        777,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };
    print_stats(arr);
}