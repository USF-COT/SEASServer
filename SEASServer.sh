#! /bin/sh
### BEGIN INIT INFO
# Provides:          seas_server
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: SEAS Server
# Description:       Service used to communicate between GUI clients and LON
### END INIT INFO

# Author: Michael Lindemuth <mlindemu@usf.edu>
#

# Do NOT "set -e"

# PATH should only include /usr/* if it runs after the mountnfs.sh script
PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin
DESC="Service used to communicate between SEAS GUI and instrument LON"
NAME=SEASServer
DAEMON=/usr/local/bin/$NAME
DAEMON_ARGS=""
PIDFILE=/var/run/$NAME.pid
SCRIPTNAME=/etc/init.d/$NAME

# Exit if the package is not installed
[ -x "$DAEMON" ] || exit 0

# Read configuration variable file if it is present
[ -r /etc/default/$NAME ] && . /etc/default/$NAME

        # Check and Try to Fix Spectrometer Connection If Necessary
        NUM_TRIES=10
        NUM_SPECS=`lsusb -v | grep Ocean | wc -l`
        echo $NUM_SPECS

        while [ $NUM_SPECS -lt 2 ]; do
            echo 0 > /sys/class/gpio/gpio168/value
            sleep 10
            echo 1 > /sys/class/gpio/gpio168/value
            sleep 10
            NUM_SPECS=`lsusb -v | grep Ocean | wc -l`
            NUM_TRIES=`expr $NUM_TRIES - 1`
            echo $NUM_SPECS
            if [ $NUM_TRIES -lt 0 ]; then
                logger -t SPECERROR Could Not Successfully Find Spectrometers
                break
            fi
        done

	# Return
	#   0 if daemon has been started
	#   1 if daemon was already running
	#   2 if daemon could not be started
	if [ $NUM_SPECS -gt 1 ]; then
	$DAEMON \
		|| return 1
        else
            return 2
       fi
# Add code here, if necessary, that waits for the process to be ready
# to handle requests from services started subsequently which depend
# on this one.  As a last resort, sleep for some time.

