#!/bin/bash

inhibitor=false;

while [ $# -gt 0 ] ; do
  case $1 in
    -e | --explode)
      if [ -z ${config+x} ]; then
       config=explode
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -t | --timeout)
      if [ -z ${config+x} ]; then
       config=timeout
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -m | --meltdown)
      if [ -z ${config+x} ]; then
       config=meltdown
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -b | --blackout)
      if [ -z ${config+x} ]; then
       config=blackout
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -i | --inhibitor)
      inhibitor=true
      ;;
  esac
  shift
done

make
cd bin || exit
# assicurarsi che un config sia stato selezionato e farne il source

if $inhibitor ; then
  ./master --inhibitor
else
  ./master
fi