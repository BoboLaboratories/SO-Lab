LIB_DIR=lib
OUT_BIN=bin
OUT_LIB=$(OUT_BIN)/$(LIB_DIR)

override CFLAGS += -Wvla -Wextra -Werror -D_GNU_SOURCE
export CC=gcc

# $@ target
# $^ prerequisites
# $< first prerequisite

LIBS_FULL_PATHS = $(addprefix $(OUT_LIB)/,$(LIBS))
MAIN_FULL_PATHS = $(addprefix $(OUT_BIN)/,$(MAIN))

# Collection of common libraries used by any type of process
LIBS = console shmem util fifo lifo sem signal
MAIN = master alimentatore attivatore atomo sandbox

# Directive for building the whole project
all: $(MAIN_FULL_PATHS)

# Directive for building all the libraries
libs: $(LIBS_FULL_PATHS)

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
$(OUT_BIN)/%: %/*.c model/*.c $(LIBS_FULL_PATHS)
	$(eval DEF := $(shell tr '[:lower:]' '[:upper:]' <<< $*))
	$(CC) $(CFLAGS) -D$(DEF) $(filter %.c,$^) -o $@ -L$(OUT_LIB) $(addprefix -l:,$(LIBS)) -lm

# Directive for making any library
$(OUT_LIB)/%: $(LIB_DIR)/%/*.c | $(OUT_LIB)
	$(CC) $(CFLAGS) -c -o $@ $<

# Creates the output directory
$(OUT_LIB):
	@mkdir -p $(OUT_BIN)/$(LIB_DIR)

.SILENT: all debug clean