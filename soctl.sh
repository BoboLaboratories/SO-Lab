#!/bin/bash

inhibitor=false;

while [ $# -gt 0 ] ; do
  case $1 in
    -e | --explode)
      if [ -z ${config+x} ]; then
       config=env/explode.sh
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -t | --timeout)
      if [ -z ${config+x} ]; then
       config=env/timeout.sh
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -m | --meltdown)
      if [ -z ${config+x} ]; then
       config=env/meltdown.sh
      else
        echo 'ERROR'
        exit 1
      fi
      ;;
    -b | --blackout)
      if [ -z ${config+x} ]; then
       config=env/blackout.sh
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

if $inhibitor ; then
  ./master --inhibitor
else
  ./master
fi