rm .so_fifo
truncate -s 0 exits.txt
source "../../env/timeout.sh"
gdb -x ../../debug.txt
