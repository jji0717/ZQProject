#!/bin/bash
#
# chkconfig: 345 30 80
# description: A fuse filesystem maps to a CDMI-capatible storage

# should be started prior to samba and stopped after it, which usually takes S52 and K50

INST_HOME=/opt/cdmifuse

# Source function library.
if [ -f /etc/init.d/functions ] ; then
  . /etc/init.d/functions
elif [ -f /etc/rc.d/init.d/functions ] ; then
  . /etc/rc.d/init.d/functions
elif [ -f $INST_HOME/etc/initd_funcs ] ; then
  . $INST_HOME/etc/initd_funcs
else
  echo -n "failed to load initd functions"
  exit 1
fi

RETVAL=0

PROP=$INST_HOME/bin/cdmifuse
OPTIONS=$INST_HOME/etc/cdmifuse.conf
PIDFILE=/var/run/cdmifuse_shell.pid

start() {
	echo  $"Starting cdmifuse services: "
	daemon $INST_HOME/bin/fuseshell.py $PROP $OPTIONS
	RETVAL=$?
	echo $"Starting $KIND services: "
	return $RETVAL
}	

stop() {
	echo  $"Shutting down cdmifuse services: "
	killproc -p $PIDFILE python
	RETVAL=$?
	echo ""
	return $RETVAL
}	

restart() {
	stop
	start
}	

rhstatus() {
	status cdmifuse
	RETVAL=$?
	if [ $RETVAL -ne 0 ] ; then
		return $RETVAL
	fi
}	

# Allow status as non-root.
if [ "$1" = status ]; then
       rhstatus
       exit $?
fi


case "$1" in
  start)
  	start
	;;
  stop)
  	stop
	;;
  restart)
  	restart
	;;
  reload)
  	reload
	;;
  status)
  	rhstatus
	;;
  *)
	echo $"Usage: $0 {start|stop|restart|status}"
	exit 2
esac

exit $?
