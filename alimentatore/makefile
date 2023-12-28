../bin/alimentatore: \
	../bin/libs/console.o \
	../bin/libs/shmem.o \
	../bin/libs/util.o \
	../bin/libs/fifo.o \
	../bin/libs/sem.o \
	../model/model.c ../model/model.h \
	alimentatore.c
	$(CC) $(CFLAGS) -DALIMENTATORE $(filter-out %.h,$^) -o $@
