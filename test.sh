#!/bin/bash

# ./text.sh <scenario> [--inhibitor]
dir="test/$1$2"

make clean
make

mkdir -p "$dir"
cp bin/* "$dir"

cd "$dir" || exit 1

touch start.sh
touch exits.txt
echo "rm .so_fifo" >> start.sh
echo "truncate -s 0 exits.txt" >> start.sh
echo "source \"../../env/$1.sh\"" >> start.sh
echo "gdb -x ../../debug.txt" >> start.sh
chmod u+x start.sh