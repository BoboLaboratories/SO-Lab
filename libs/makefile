all: ../bin/libs/util.o ../bin/libs/ipc.o ../bin/libs/lifo.o

../bin/libs/lifo.o: lifo/lifo.c lifo/lifo.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/util.o: util/util.c util/util.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/ipc.o: ipc/ipc.c ipc/ipc.h
	$(CC) $(CFLAGS) -c -o $@ $<