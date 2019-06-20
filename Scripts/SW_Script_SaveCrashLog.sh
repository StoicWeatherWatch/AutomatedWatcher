#!/bin/bash

# SW_Script_SaveCrashLog.sh
# 
# 2019-6-19

# This script will check to see if WeeWX is running and preserve the sys log if
# it has crashed. It will copy /var/log/syslog to ~/
# 

# WeeWX Process name. weewxd is the daemon
WeeWXName="weewxd"

# Fail Log File
SysLogFileDest=/home/pi/

# Current time and date
DATE=`date +%Y-%m-%d`
TIME=`date +%H-%M`

# Check if WeeWx is not running
if ! pgrep -x $WeeWXName > /dev/null 2>&1
then
# Not Running - copy log
cp /var/log/syslog ${SysLogFileDest}${DATE}_${TIME}_syslogCopy.txt
fi
