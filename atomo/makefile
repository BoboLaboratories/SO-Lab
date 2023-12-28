../bin/atomo: \
	../bin/libs/console.o \
	../bin/libs/shmem.o \
	../bin/libs/util.o \
	../bin/libs/lifo.o \
	../bin/libs/sem.o \
	../model/model.c ../model/model.h \
	atomo.c
	$(CC) $(CFLAGS) -DATOMO $(filter-out %.h,$^) -o $@
