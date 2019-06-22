#!/usr/bin/env python
#
# SW Remote Watcher
# 2019-6-22
#
""" A class to monitor remote sensors and report the date when asked. 
Remotes will push data. This class will hold it and report it when asked
"""

#TODO Try a sequence of addresses (ports) until one works

from twisted.protocols import amp
from twisted.internet import reactor
from twisted.internet import error as twistedError
from twisted.internet.protocol import Factory

import threading

import traceback
import syslog
#import time


def logmsg(level, msg):
    try:
	syslog.syslog(level, 'SW_RemoteWatcher: %s' % msg)
    except TypeError as detail:
	# This happened onece on daemon stop. Well into a stop that seemed to be going well,
	#  a line was read in from serial and sent to loginf. Syslog spit out a TypeError
	#  and the program crashed and would not come back to life automatically.
	syslog.syslog(syslog.LOG_ERR, "SW_RemoteWatcher: ERROR: syslog spit a TypeError exception")
	syslog.syslog(syslog.LOG_ERR, "SW_RemoteWatcher: ERROR: Usually means string input has issues or is none")
	syslog.syslog(syslog.LOG_ERR, "SW_RemoteWatcher: ERROR: TypeError: %s" % detail)
	syslog.syslog(syslog.LOG_ERR, "SW_RemoteWatcher: Traceback: \n%s" % traceback.format_exc())
	syslog.syslog(syslog.LOG_INFO, "SW_RemoteWatcher: Stoic will now try to put this behind it and continue with life")
    except:
	syslog.syslog(syslog.LOG_ERR, "SW_RemoteWatcher: ERROR: syslog spit an error other than TypeError exception. (Raised to next level)")
	syslog.syslog(syslog.LOG_ERR, "SW_RemoteWatcher: Traceback: \n%s" % traceback.format_exc())
	raise

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, 'ERROR: %s' % msg)
    
class RemoteWatcherFactory(Factory):
    def __init__(self, RemoteWatcherInstance):
        loginf("RemoteWatcherFactory __init__")
        
        self.RemoteWatcherInstance = RemoteWatcherInstance
        #super(RemoteWatcherFactory, self).__init__() Factory has no init last I checked
        

        
        loginf("RemoteWatcherFactory __init__ done")
    
    def ReportTheDataToWeeWX(self, data):
        self.RemoteWatcherInstance.ReportTheDataToWeeWX(data)
    
    
    
class IndoorPiA(amp.Command):
    arguments = [('TA', amp.Float()),
                 ('PA', amp.Float()),
                 ('HA', amp.Float())]
    response = [('cksu', amp.Float())]
    


class SW_RemoteWatcher(object):
    """StoicWSDriver in StoicWS will hold an instance of this."""

    def __init__(self, portNum,ExpectedDataDict):
        loginf("SW_RemoteWatcher __init__")
        
        self.portNum = portNum
        loginf("portNum %d" %self.portNum)
        
        #TODO Do something with this
        self.ExpectedDataDict = ExpectedDataDict
        
        self.Data = None
        self.NewData = False
        self.SetupFailed = False
        
        # For the communication
        self.factory = RemoteWatcherFactory(self)
        loginf("SW_RemoteWatcher setting factory.protocol = SW_RemoteAMPReceiver")
        self.factory.protocol = SW_RemoteAMPReceiver
        loginf("SW_RemoteWatcher reactor.listenTCP ")
        try:
            reactor.listenTCP(self.portNum,self.factory)
            
        except twistedError.CannotListenError:
            loginf("SW_RemoteWatcher reactor.listenTCP sent twisted.internet.error.CannotListenError  ")
            loginf(traceback.format_exc())
            self.SetupFailed = True
        except:
            loginf("SW_RemoteWatcher reactor.listenTCP some other error")
            loginf(traceback.format_exc())
            self.SetupFailed = True
        
        #else:
            # Only attempt to run if listen works
            #loginf("SW_RemoteWatcher reactor.run ")
            #try:
             #   reactor.run()
            #except:
             #   loginf("SW_RemoteWatcher reactor.run() some other error")
             #   loginf(traceback.format_exc())
             #   self.SetupFailed = True
        loginf("SW_RemoteWatcher __init__ done")
        
    def StartMonitoringForRemotes(self):
        try:
            loginf("SW_RemoteWatcher StartMonitoringForRemotes Starting")
            self.ThreadForTwisted = threading.Thread(target=reactor.run, name="nThreadForTwisted")
            loginf("SW_RemoteWatcher StartMonitoringForRemotes self.ThreadForTwisted.start()")
            self.ThreadForTwisted.start()
            loginf("SW_RemoteWatcher StartMonitoringForRemotes Done")
            return True
        except:
            loginf("SW_RemoteWatcher StartMonitoringForRemotes reactor.run() some error")
            loginf(traceback.format_exc())
            self.StartFailed = True
            return False
        
    def StopMonitoringForRemotes(self):
        loginf("SW_RemoteWatcher StopMonitoringForRemotes called but has issues - might want to fix this someday")
        #TODO Make this work
        try:
            loginf("SW_RemoteWatcher StopMonitoringForRemotes Running")
            reactor.callFromThread(reactor.stop)
            loginf("SW_RemoteWatcher StopMonitoringForRemotes Done")
            return True
        except:
            loginf("SW_RemoteWatcher StopMonitoringForRemotes some error")
            loginf(traceback.format_exc())
            #This kills the thread brutaly
            self.ThreadForTwisted.exit()
            return False
    
    def ReportTheDataToWeeWX(self, data):
        """
        This is called whenever new data is received from the remote
        """
        self.NewData = True
        self.Data = data
        
        #loginf("SW_RemoteWatcher ReportTheDataToWeeWX reactor.callFromThread(reactor.stop)")
        #reactor.callFromThread(reactor.stop)
        
        # TODO make this more robust against bad data
        loginf("SW_RemoteWatcher ReportTheDataToWeeWX New Data from Indoors T %f P %f H %f" %(data.get('TA'),data.get('PA'),data.get('HA')))
        
    def GetData(self):
        """
        This is called from stoic. We only send data once.
        """
        
        loginf("SW_RemoteWatcher GetData")
        
        if self.NewData:
            loginf("SW_RemoteWatcher GetData has new data")
            self.NewData = False
            ConvertedData = self.ConvertKeys(self.Data)
            
            #loginf("SW_RemoteWatcher ReportTheDataToWeeWX self.StartMonitoringForRemotes()")
            #self.StartMonitoringForRemotes()
            
            return ConvertedData
        else:
            return None
        
    def ConvertKeys(self,data):
        DataOut = dict()
        # TODO make this less hard coded
        DataOut["TempHouse1"] = data.get('TA')
        DataOut["HumidityHouse1"] = data.get('HA')
        DataOut["pressureHouse1"] = data.get('PA')
        
        return DataOut
    
        
#class SW_ReceiverThread(threading.Thread):
    
        



class SW_RemoteAMPReceiver(amp.AMP):
    """This handles the actuall communication"""

    #@IndoorPiA.responder
    def indoorPiA(self, TA,PA,HA):
        """
        This is called by magic whenever there is new data
        """
        loginf( "SW_RemoteAMPReceiver indoorPiA " )
        
        loginf( "SW_RemoteAMPReceiver indoorPiA T %f P %f H %f" %(TA,PA,HA)) 
        
        data = dict()
        data["TA"] = TA
        data["PA"] = PA
        data["HA"] = HA
        
        self.factory.ReportTheDataToWeeWX(data)

        # TODO add checksum
        return {'cksu': 0.0}
    IndoorPiA.responder(indoorPiA)

    def onInsertError(self, error, data):
        """
        Here you could do some additional error checking or inspect data 
        which was handed for insert here. For now we will just throw the same exception again
        so that the client gets notified
        """
        loginf("SW_RemoteAMPReceiver onInsertError")
        raise error
    


