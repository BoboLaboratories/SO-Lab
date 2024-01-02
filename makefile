override CFLAGS += -Wvla -Wextra -Werror -D_GNU_SOURCE
export CC=gcc

# $@ target
# $^ prerequisites
# $< first prerequisite

LIBS_FULL_PATHS = $(addprefix bin/libs/,$(LIBS))
MAIN_FULL_PATHS = $(addprefix bin/,$(MAIN))

# Collection of common libraries used by any type of process
LIBS = console shmem util fifo lifo sem sig
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
bin/%: %/*.c model/model.c $(LIBS_FULL_PATHS)
	$(eval DEF := $(shell tr '[:lower:]' '[:upper:]' <<< $*))
	$(CC) $(CFLAGS) -D$(DEF) $(filter %.c,$^) -o $@ -Ilibs -Imodel -Lbin/libs -lm $(addprefix -l:,$(LIBS))

# Directive for making any library
bin/libs/%: libs/impl/%.c | makedir
	$(CC) $(CFLAGS) -c -o $@ $< -Ilibs

# Creates the output directory
makedir:
	@mkdir -p bin/libs

.SILENT: all debug clean