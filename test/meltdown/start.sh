rm .so_fifo
truncate -s 0 exits.txt
source "../../env/meltdown.sh"
gdb -x ../../debug.txt
