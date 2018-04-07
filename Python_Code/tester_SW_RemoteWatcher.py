#tester_SW_RemoteWatcher.py

from SW_RemoteWatcher import SW_RemoteWatcher
import time



def loginf(msg):
    print('tester_SW_RemoteWatcher: %s' % msg)


    
loginf("starting")

loginf("opening SW_RemoteWatcher")
RemoteSouce = SW_RemoteWatcher(1216,dict())
loginf(" StartMonitoringForRemotes()")
RemoteSouce.StartMonitoringForRemotes()

for i in range(30):
    loginf("Looping")
    data2 = RemoteSouce.GetData()
    if data2 != None:
        loginf("Data is not None")
        print(data2)
    else:
        loginf("Data is None")
        print(data2)
        
    time.sleep(2)
    
exit()