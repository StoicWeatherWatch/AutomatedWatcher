#!/bin/bash

# SW_Script_BackDB.sh
# Joseph Wellhouse
# 2019-6-17

# This script will stop WeeWX, copy the database archive, sync the source files, and restart the Pi.
### Restart does not currently work
# It is designed to be run once per day.
# The best time for this is between Midnight and 1 or between 3 and 4 to avoid daylight savings time issues.
# Most likely needs to run as root
# The copy will be given the date as a prefix. Two different log files are kept. One for fail
# on the main drive and one for success on the backup drive.

# Cron setup
# sudo crontab -e
#42 0 * * *    /home/pi/ScriptsForWeeWX/SW_Script_BackDB.sh
# sudo crontab -l

# User and group not public
# chmod 770 SW_Script_BackDB.sh

# Path to backup directory
BackupPath=/mnt/StoicBak/
BackupSubDir=StoicBackups/
SourceBackupSubDir=StoicSourceBackup/
TestFileName=StoicBackisMounted

# Path to WeeWx DB
DBPath=/home/weewx/archive/

# Paths to WeeWX driectories to be synced
WeeWXDir=/home/weewx/
WeeWXSourceDirList=( "bin/" "skins/PlaceHolder/" )

# WeeWX DB File
WeeWXDBName=weewx.sdb

# Fail Log File
LogFileFail=/home/pi/BackupFailLog.txt

# Success log file
LogFileSuccess=${BackupPath}BackupSuccessLog.txt

# rsync log file
LogFileRsync=${BackupPath}StoicRsyncLog.txt

# WeeWX Process name
WeeWXName="weewxd"

# Current time and date
DATE=`date +%Y-%m-%d`
TIME=`date +%H-%M`

# Check if WeeWx is not running
if ! pgrep -x $WeeWXName > /dev/null 2>&1
then
# Not running - write to log
printf "$DATE  $TIME  ERROR WeeWX not running\n" >> $LogFileFail
fi

# Stop WeeWx

/etc/init.d/weewx stop

# Check that the back up drive is mounted by checking for a file
# If not
if ! [ -f "$BackupPath$TestFileName" ];
then
# Fail, log, and die
printf "$DATE  $TIME  ERROR Backup directory not mounted\n" >> $LogFileFail
exit 5
fi

# Check if WeeWx is still running
if pgrep -x $WeeWXName > /dev/null 2>&1
then
# Still Running - Sleep and try again
# TEMP
echo "Still running"
sleep 2
if pgrep -x $WeeWXName > /dev/null 2>&1
then
# Still Running - Sleep and try again
# TEMP
echo "Still running after 2"
sleep 5

if pgrep -x $WeeWXName > /dev/null 2>&1
then
# Still Running - Fail and die
printf "$DATE  $TIME  ERROR WeeWX failed to quit\n" >> $LogFileFail
exit 10
fi

fi
fi



# Copy DB acrhive
# Prefix will be the date
# Check to see if the file exists
if [ ! -f ${BackupPath}${BackupSubDir}${DATE}_${WeeWXDBName} ];
then

cp ${DBPath}${WeeWXDBName} ${BackupPath}${BackupSubDir}${DATE}_${WeeWXDBName}
CPExitCode=$?
# Spaces required [$CPExitCode -eq 0] will not work.
# -eq is numeric ==
if [ $CPExitCode -eq 0 ]
then
printf "$DATE  Backup made with cp exit code $CPExitCode\n" >> $LogFileSuccess
else
printf "$DATE  $TIME  ERROR Backup failed. cp exit code $CPExitCode\n" >> $LogFileFail
fi

else
printf "$DATE  $TIME  ERROR Backup failed. File exists\n" >> $LogFileFail
# Note that this means the code will stop and not back up the source if this error happens
exit 15
fi






# Sync WeeWx.conf file
rsync --inplace --log-file=${LogFileRsync} ${WeeWXDir}weewx.conf ${BackupPath}${SourceBackupSubDir}
rsyncExitCode=$?
if [ $rsyncExitCode -ne 0 ]
then
printf "$DATE  $TIME  ERROR Sync failed on weewx.conf - rsync exit code $rsyncExitCode\n" >> $LogFileFail
fi


# Sync the WeeWx source files
for SourceDir in "${WeeWXSourceDirList[@]}"
do
rsync -r --inplace --log-file=${LogFileRsync} ${WeeWXDir}${SourceDir} ${BackupPath}${SourceBackupSubDir}${SourceDir}
rsyncExitCode=$?
if [ $rsyncExitCode -ne 0 ]
then
printf "$DATE  $TIME  ERROR Sync failed on ${SourceDir} - rsync exit code $rsyncExitCode\n" >> $LogFileFail
fi
done

# rsync
# --inplace skips temp file creation and speeds things up.
# --log-file=FILE
# --itemize-changes List the changes made

# Restart the Pi
# cron does not have a full set of paths only  /usr/bin:/bin thus we need to give the full path to shutdown
/sbin/shutdown -r now


# Next Script will:
# Second script will check for WeeWx running and write to sucess or fail log
