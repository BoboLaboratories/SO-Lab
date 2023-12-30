LIB_DIR=libs
OUT_BIN=bin
OUT_LIB=$(OUT_BIN)/$(LIB_DIR)

export CC=gcc
export LD_LIBRARY_PATH+=$(OUT_LIB)
override CFLAGS += -Wvla -Wextra -Werror -D_GNU_SOURCE

# $@ target
# $^ prerequisites
# $< first prerequisite

# Collection of common libraries used by any type of process
LIBS = console shmem util fifo lifo sem
MAIN = master alimentatore attivatore atomo

# Directive for building the whole project
all: $(addprefix $(OUT_BIN)/,$(MAIN))

# Directive for building all the libraries
libs: $(addprefix $(OUT_LIB)/,$(LIBS))

# Directive for building the whole project with full debug enabled
# Partial debug can be enabled like so
#	$ make CFLAGS="-DD_<component>"
# passing as many component as needed e.g.
#	$ make CFLAGS="-DD_SHMEM -DD_FIFO"
debug: CFLAGS += -DDEBUG
debug: all

# Simple clean directive
clean:
	echo -n "Cleaning up.."
	rm -rf bin
	echo " done."

# Directive for building any main component
$(OUT_BIN)/%: %/* model/* libs/* libs
	$(eval DEF := $(shell tr '[:lower:]' '[:upper:]' <<< $*))
	$(CC) $(CFLAGS) -D$(DEF) $(filter %.c,$^) -o $@ -L$(OUT_LIB) $(addprefix -l:,$(LIBS))

# Directive for making any library
$(OUT_LIB)/%: $(LIB_DIR)/%/*.c $(LIB_DIR)/%/*.h | $(OUT_LIB)
	$(CC) $(CFLAGS) -c -o $@ $<

# Creates the output directory
$(OUT_LIB):
	@mkdir -p bin/libs

.SILENT: all debug clean