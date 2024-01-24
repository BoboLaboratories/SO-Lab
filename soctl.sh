#!/bin/bash

#soctl run --meltdown --inhib
#    source env/meltdown.sh
#    ./master_pid --inhib
#
#soctl stop
#    kill -SIGTERM $(preg master_pid)
#
#soctl inhibitor <start/stop/toggle>
#    start  = ./inhibitor_ctl 1
#    stop   = ./inhibitor_ctl 0
#    toggle = ./inhibitor_ctl



# soctl start -e --inhibitor
function start() {
  inhibitor=
  config=

  function select_config() {
    if [ -z "$config" ]; then
     config="env/$1.sh"
    else
      echo 'ERROR: can only select one ending scenario!'
      exit 1
    fi
  }

  while [ $# -gt 0 ] ; do
    case $1 in
      -e | --explode)
        select_config "explode"
        ;;
      -t | --timeout)
        select_config "timeout"
        ;;
      -m | --meltdown)
        select_config "meltdown"
        ;;
      -b | --blackout)
        select_config "blackout"
        ;;
      -i | --inhibitor)
        inhibitor=--inhibitor
        ;;
      *)
        echo "ERROR: invalid option $1!"
        exit 1
    esac
    shift
  done

  if [ -z "$config" ]; then
    echo "ERROR: no ending scenario was selected!"
    exit 1
  fi

  source "$config"
  make clean
  make
  cd bin || exit 1
  ./master $inhibitor
  exit 0
}

function stop() {
  pid=$(pgrep --newest master)
  if [ -z "$pid" ]; then
    echo "ERROR: no simulation is running!"
    exit 1
  else
    kill -SIGTERM "$pid"
    echo "Simulation terminated!"
    exit 0
  fi

  exit 0
}

function inhibitor() {
  op=

  function select_op() {
    if [ -z "$op" ]; then
     op=$1
    else
      echo 'ERROR: can only perform a single operation!'
      exit 1
    fi
  }

  while [ $# -gt 0 ] ; do
    case $1 in
      start) select_op "start"
      ;;
      stop) select_op "stop"
      ;;
      toggle) select_op "toggle"
      ;;
      *)
        echo "ERROR: invalid option $1!"
        exit 1
    esac
    shift
  done

  cd bin || exit 1
  ./inhibitor_ctl "$op"
  exit 0
}


while [ $# -gt 0 ] ; do
  case $1 in
    start)
      shift
      start "$@"
    ;;
    stop)
      stop
    ;;
    inhibitor)
      shift
      inhibitor "$@"
    ;;
    *)
      echo "ERROR: invalid operation $1!"
      exit 1
    ;;
  esac
done

# TODO
# new terminal per simulazione start
# finire inhibitor ctl
# errore/help se comandi errati

#make
#cd bin || exit
## assicurarsi che un config sia stato selezionato e farne il source
#
#if $inhibitor ; then
#  ./master --inhibitor
#else
#  ./master
#fi