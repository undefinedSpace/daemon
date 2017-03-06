#!/usr/bin/env bash

### BEGIN INIT INFO
# Provides:          usd
# Required-Start:    $local_fs $network $named $time $syslog
# Required-Stop:     $local_fs $network $named $time $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Description:       Bot for Discord
### END INIT INFO

FILE="/usr/sbin/usd"
RUNAS="root"
NAME="usd"

PIDFILE=/var/run/$NAME.pid
LOGFILE=/var/log/$NAME.log

touch $LOGFILE
chown $RUNAS $LOGFILE

start() {
  if [ -f /var/run/$PIDNAME ] && kill -0 $(cat /var/run/$PIDNAME); then
    echo 'Service already running' >&2
    return 1
  fi
  echo -n 'Starting service... ' >&2
  local CMD="$FILE &> \"$LOGFILE\" & echo \$!"
  su -c "$CMD" $RUNAS > "$PIDFILE"
  echo 'done' >&2
}

stop() {
  if [ ! -f "$PIDFILE" ] || ! kill -0 $(cat "$PIDFILE"); then
    echo 'Service not running' >&2
    return 1
  fi
  echo -n 'Stopping service... ' >&2
  kill -15 $(cat "$PIDFILE") && rm -f "$PIDFILE"
  echo 'done' >&2
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  restart)
    stop
    start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart}"
esac
