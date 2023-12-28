override CFLAGS += -Wvla -Wextra -Werror -D_GNU_SOURCE
export CC=gcc

# $@ target
# $^ prerequisites
# $< first prerequisite

OUT_BIN=bin
OUT_LIB=$(OUT_BIN)/libs

# Directive for building the whole project
all: 						\
	$(OUT_BIN)/master 		\
	$(OUT_BIN)/alimentatore	\
	$(OUT_BIN)/attivatore	\
	$(OUT_BIN)/atomo		\

# Directive for building the whole project with debug enabled
debug: CFLAGS+=-DDEBUG -DD_FIFO -DD_SHMEM
debug: all

# Directive for making any library
$(OUT_LIB)/%.o: libs/%/*.c libs/%/*.h | $(OUT_LIB)
	$(CC) $(CFLAGS) -c -o $@ $<

# Collection of common libraries used by any type of process
$(OUT_LIB)/common.o: 		\
	$(OUT_LIB)/console.o	\
	$(OUT_LIB)/shmem.o		\
	$(OUT_LIB)/util.o		\
	$(OUT_LIB)/sem.o
	ld -relocatable $^ -o $@

$(OUT_BIN)/master: 			\
	$(OUT_LIB)/common.o 	\
	$(OUT_LIB)/fifo.o 		\
	model/* 				\
	master/*
	$(CC) $(CFLAGS) -DMASTER $(filter-out %.h,$^) -o $@

$(OUT_BIN)/alimentatore:	\
	$(OUT_LIB)/common.o 	\
	$(OUT_LIB)/fifo.o 		\
	model/* 				\
	alimentatore/*
	$(CC) $(CFLAGS) -DALIMENTATORE $(filter-out %.h,$^) -o $@

$(OUT_BIN)/attivatore:		\
	$(OUT_LIB)/common.o 	\
	$(OUT_LIB)/fifo.o 		\
	$(OUT_LIB)/lifo.o		\
	model/* 				\
	attivatore/*
	$(CC) $(CFLAGS) -DATTIVATORE $(filter-out %.h,$^) -o $@

$(OUT_BIN)/atomo:			\
	$(OUT_LIB)/common.o 	\
	$(OUT_LIB)/lifo.o		\
	model/* 				\
	atomo/*
	$(CC) $(CFLAGS) -DATOMO $(filter-out %.h,$^) -o $@

clean:
	echo -n "Cleaning up.."
	rm -rf bin
	echo " done."

$(OUT_LIB):
	@mkdir -p bin/libs

.SILENT: all debug clean