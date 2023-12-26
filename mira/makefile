export CC=gcc
export CFLAGS=-Wvla -Wextra -Werror -D_GNU_SOURCE

all: bin/master bin/inhibitor bin/atom

clean:
	rm -r bin
	mkdir -p bin/libs

bin/master: bin/libs/mira.o master.c
	$(CC) $(CFLAGS) $^ -o $@

bin/inhibitor: bin/libs/mira.o inhibitor.c
	$(CC) $(CFLAGS) $^ -o $@

bin/atom: bin/libs/mira.o atom.c
	$(CC) $(CFLAGS) $^ -o $@

bin/libs/mira.o: mira.c mira.h
	$(CC) $(CFLAGS) -c -o $@ $<