../bin/master: \
	../bin/libs/console.o \
	../bin/libs/shmem.o \
	../bin/libs/util.o \
	../bin/libs/fifo.o \
	../bin/libs/sem.o \
	../model/model.c \
	../model/model.h \
	config.c config.h \
	cleanup.c cleanup.h \
	master.c
	$(CC) $(CFLAGS) -DMASTER $(filter-out %.h,$^) -o $@
