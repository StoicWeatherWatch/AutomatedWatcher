# SW_TryToCrashWithStop.py
# This will repeatedly start and stop weewx to reproduce crash on stop situations.
# It test from startFloat to 10 seconds longer at 0.1 sec increments
# 2019-6-22

import os
import time

# Seconds to start from
StartFloat = 10.0

Command = "sudo /etc/init.d/weewx stop"
ResetCommand = "sudo /etc/init.d/weewx start"

for i in range(0,100):
  TimeStep = StartFloat + (i/10.0)
  print(TimeStep)
  os.system(ResetCommand)
  time.sleep(TimeStep)
  os.system(Command)
