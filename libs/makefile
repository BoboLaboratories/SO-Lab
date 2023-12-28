all: \
	../bin/libs/console.o \
	../bin/libs/util.o \
	../bin/libs/shmem.o \
	../bin/libs/fifo.o \
	../bin/libs/lifo.o \
	../bin/libs/sem.o

../bin/libs/console.o: console/console.c console/console.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/util.o: util/util.c util/util.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/shmem.o: shmem/shmem.c shmem/shmem.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/fifo.o: fifo/fifo.c fifo/fifo.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/lifo.o: lifo/lifo.c lifo/lifo.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/sem.o: sem/sem.c sem/sem.h
	$(CC) $(CFLAGS) -c -o $@ $<