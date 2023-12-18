all: ../bin/libs/config.o ../bin/libs/ipc.o

../bin/libs/config.o: config/config.c config/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

../bin/libs/ipc.o: ipc/ipc.c ipc/ipc.h
	$(CC) $(CFLAGS) -c -o $@ $<