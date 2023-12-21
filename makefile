export CC=gcc
export CFLAGS=-Wvla -Wextra -Werror -D_GNU_SOURCE

MODULES = libs master alimentatore atomo

all:
	mkdir -p bin/libs
	for module in $(MODULES); do 		   	\
		echo "Building module $$module.."; 	\
		$(MAKE) -s -C $$module;				\
	done

debug: CFLAGS += -DDEBUG
debug: all

clean:
	echo -n "Cleaning up.."
	rm -rf bin
	echo " done."

.SILENT: all debug clean