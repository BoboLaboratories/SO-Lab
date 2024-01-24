override CFLAGS += -Wvla -Wextra -Werror -D_GNU_SOURCE
export CC=gcc

# $@ target
# $^ prerequisites
# $< first prerequisite

# Collection of common libraries used by any type of process
LIBS = console shmem util fifo lifo sem sig
MAIN = master alimentatore attivatore atomo inibitore sandbox

# Paths to all executables and libraries
LIBS_FULL_PATHS = $(addprefix bin/libs/,$(LIBS))
MAIN_FULL_PATHS = $(addprefix bin/,$(MAIN))
MAIN_FULL_PATHS += bin/inhibitor_ctl
HEADER_DIRECTORIES = libs model

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

# Directive for building inhibitor ctl
bin/inhibitor_ctl: inhibitor_ctl/inhibitor_ctl.c bin/libs/sem
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ -Ilibs -Lbin/libs -l:sem

# Directive for building any main component
bin/%: %/*.c model/model.c $(LIBS_FULL_PATHS)
	$(eval DEF := $(shell echo $* | tr '[:lower:]' '[:upper:]'))
	$(CC) $(CFLAGS) -DD_PID -D$(DEF) $(filter %.c,$^) -o $@ $(addprefix -I,$(HEADER_DIRECTORIES)) -Lbin/libs $(addprefix -l:,$(LIBS))

# Directive for making any library
bin/libs/%: libs/impl/%.c | makedir
	$(CC) $(CFLAGS) -c -o $@ $< -Ilibs

# Creates the output directory
makedir:
	@mkdir -p bin/libs

.SILENT: all debug clean