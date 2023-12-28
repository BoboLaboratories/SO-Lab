all: ../bin/libs/model.o ../bin/libs/util.o ../bin/libs/lifo.o ../bin/libs/lifo.o ../bin/libs/ipc.o ../bin/libs/sem.o

../bin/libs/model.o: model/model.c model/model.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/util.o: util/util.c util/util.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/fifo.o: fifo/fifo.c fifo/fifo.c
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/lifo.o: lifo/lifo.c lifo/lifo.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/sem.o: sem/sem.c sem/sem.h
	$(CC) $(CFLAGS) -c -o $@ $<