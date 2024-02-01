#!/bin/bash
# <dir> <scenario...>
dir=$1
shift

status=
while [ $# -gt 0 ] ; do
case $1 in
  explode)
    status=3
    ;;
  timeout)
    status=2
    ;;
  meltdown)
    status=5
    ;;
  blackout)
     status=4
    ;;
  *)
    echo "ERROR: invalid option $1!"
    exit 1
esac

scenario=$(cat "$dir/exits.txt" | tr -cd "$status" | wc -c)
total=$(cat "$dir/exits.txt" | wc -l)

echo "$1: [$scenario/$total]"
shift

done