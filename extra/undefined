#!/bin/sh

### BEGIN INIT INFO
# Provides:		undefined
# Required-Start:	$network mysql $local_fs $remote_fs
# Required-Stop:	$network mysql $local_fs $remote_fs
# Default-Start:	2 3 4 5
# Default-Stop:		0 1 6
# Short-Description:	undefinedSpace
# Description:		undefinedSpace daemon
### END INIT INFO

set -e

SERVER="/usr/sbin/undefined"
NAME="undefined"
PATH="/sbin:/bin:/usr/sbin:/usr/bin"

test -x "${SERVER}" || exit 0

. /lib/lsb/init-functions

case "${1}" in
	start)
		echo "Starting ${NAME}"

		if [ ! -d /var/run/${NAME} ]
		then
		    mkdir /var/run/${NAME}
		fi

		if [ ! -f /var/run/${NAME}/${NAME}.pid ]
		then
		    :>/var/run/${NAME}/${NAME}.pid
		fi

		start-stop-daemon -c undefined --start --background --oknodo -m --pidfile /var/run/${NAME}/${NAME}.pid --exec "${SERVER}"

		if ! ps -C ${NAME} | grep -qs "${_PID}"
		then
			echo "${NAME} failed"
			exit 1
		fi

		;;
	stop)
		echo "Stopping ${NAME}, please, wait for database saving"

		start-stop-daemon --stop --pidfile /var/run/${NAME}/${NAME}.pid --retry 30 --exec ${SERVER}
		RET=$?

		if [ $RET = 0 ]; then rm -f /var/run/${NAME}/${NAME}.pid; fi
		;;

		restart)
			${0} stop
			${0} start
			;;

		status)
			status_of_proc "${SERVER}" "undefinedSpace"
			;;

		*)
			echo "Usage: ${0} {start|stop|restart|status}"
			exit 1
			;;
esac
