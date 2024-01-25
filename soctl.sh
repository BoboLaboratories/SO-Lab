#!/bin/bash

# soctl start -e --inhibitor
function start() {
  pid=$(pgrep --newest master)
  if [ -z ${pid+x} ]; then
    echo "ERROR: another simulation is !already started"
    exit 1
  fi

  inhibitor_no_log=
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
      -n | --no-inh-log)
        inhibitor_no_log=--no-inh-log
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
  make
  cd bin || exit 1
  ./master $inhibitor $inhibitor_no_log
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

function print_help() {
  echo -e "\033[0;35m"
  echo "  ____        _           _           _          "
  echo " |  _ \      | |         | |         | |         "
  echo " | |_) | ___ | |__   ___ | |     __ _| |__  ___  "
  echo " |  _ < / _ \| '_ \ / _ \| |    / _\` | '_ \/ __| "
  echo " | |_) | (_) | |_) | (_) | |___| (_| | |_) \__ \ "
  echo " |____/ \___/|_.__/ \___/|______\__,_|_.__/|___/ "



  echo -e "\e[0m"
  echo ""
  echo " [*] Starting a simulation:"
  echo "  |    Usage: ./soctl.sh start <scenario> [flags..]"
  echo "  |"
  echo "  |    where <scenario> must be exactly one of:"
  echo "  |      -m, --meltdown     selects meltdown ending scenario"
  echo "  |      -b, --blackout     selects blackout ending scenario"
  echo "  |      -t, --timeout      selects timeout ending scenario"
  echo "  |      -e, --explode      selects explode ending scenario"
  echo "  |"
  echo "  |    and [flags..] can optionaly be none or many in:"
  echo "  |      -i, --inhibitor    starts the simulations with inhibitor activated"
  echo "  |      -n, --no-inh-log   disables inhibitor logs during the simulation"
  echo "  *"
  echo ""
  echo " [*] Stopping a simulation:"
  echo "  |    Usage: ./soctl.sh stop"
  echo "  *"
  echo ""
  echo " [*] Managing the inhibitor during a simulation:"
  echo "  |    Usage: ./soctl.sh inhibitor <operation>"
  echo "  |"
  echo "  |    where <operation> can be any of the following:"
  echo "  |      toggle             toggles the inhibitor"
  echo "  |      start              starts the inhibitor"
  echo "  |      stop               stops the inhibitor"
  echo "  *"
  echo ""
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
    --help)
      print_help
      exit 0
    ;;
    *)
      echo "ERROR: invalid operation $1!"
      exit 1
    ;;
  esac
done