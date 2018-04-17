"""
SW_Engine
2018-04-07
In which we play with fire
Used to subcalss and override certain services. 



"""


import syslog



def logmsg(level, msg):
    syslog.syslog(level, 'SW_Engine: %s' % msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, 'ERROR: %s' % msg)


        