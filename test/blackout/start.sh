rm .so_fifo
truncate -s 0 exits.txt
source "../../env/blackout.sh"
gdb -x ../../debug.txt
