override CFLAGS += -Wvla -Wextra -Werror -D_GNU_SOURCE -DDEBUG
export CC=gcc

# $@ target
# $^ prerequisites
# $< first prerequisite

# All modules and libraries
MAIN = master alimentatore attivatore atomo inibitore
LIBS = console shmem util fifo lifo sem sig

# Paths to all compiled libraries
LIBS_FULL_PATHS = $(addprefix bin/libs/,$(LIBS))

# Paths to all compiled main modules
MAIN_FULL_PATHS = $(addprefix bin/,$(MAIN))

# This one has its own specific build directive as it doesn't need model
MAIN_FULL_PATHS += bin/inhibitor_ctl

# Directories in which to look for headers
HEADER_DIRECTORIES = libs model

# Directive for building the whole project
all: $(MAIN_FULL_PATHS)

# Directive for building all the libraries
libs: $(LIBS_FULL_PATHS)

# Directive for building inhibitor ctl
bin/inhibitor_ctl: inhibitor_ctl/inhibitor_ctl.c bin/libs/sem bin/libs/console
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ -Ilibs -Lbin/libs -l:sem -l:console

# Directive for building any main component
bin/%: %/*.c model/model.c $(LIBS_FULL_PATHS)
	$(eval DEF := $(shell echo $* | tr '[:lower:]' '[:upper:]'))
	$(CC) $(CFLAGS) -D$(DEF) $(filter %.c,$^) -o $@ $(addprefix -I,$(HEADER_DIRECTORIES)) -Lbin/libs $(addprefix -l:,$(LIBS))

# Directive for making any library
bin/libs/%: libs/impl/%.c | makedir
	$(CC) $(CFLAGS) -c -o $@ $< -Ilibs

# Simple clean directive
clean:
	echo -n "Cleaning up.."
	rm -rf bin
	echo " done."

# Creates the output directory
makedir:
	@mkdir -p bin/libs

.SILENT: all debug clean