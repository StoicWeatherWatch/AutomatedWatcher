#!/bin/bash

# SW_Script_CheckRunning.sh
# Joseph Wellhouse
# 2019-6-17

# This script will check to see if WeeWX is running and spit errors if it is not.
# It is designed to be run after the reboot at the end of SW_Script_BackDB.sh.
# It usess the same log files.

# WeeWX Process name. weewxd is the daemon
WeeWXName="weewxd"

# Fail Log File
LogFileFail=/home/pi/BackupFailLog.txt

# Success log file
BackupPath=/mnt/StoicBak/
LogFileSuccess=${BackupPath}BackupSuccessLog.txt

# Current time and date
DATE=`date +%Y-%m-%d`
TIME=`date +%H-%M`

# Check that WeeWx is running
#if [ pgrep -x $WeeWXName > /dev/null 2>&1 ] # Not certain why this does not work
# if  [ pgrep -x $WeeWXName > /dev/null ] # Does nto work
#  if  pgrep -x $WeeWXName > /dev/null works but will spit errors
# > /dev/null 2>&1 sends standard out and standard error to null
# If you want logical not
# if ! pgrep -x $WeeWXName > /dev/null 2>&1
if pgrep -x $WeeWXName > /dev/null 2>&1
then
# Running - write to log
printf "$DATE  $TIME  WeeWX is running\n" >> $LogFileSuccess
else
# Not Running - write to log
printf "$DATE  $TIME  WeeWX not running\n" >> $LogFileFail
fi
