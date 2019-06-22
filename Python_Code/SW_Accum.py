"""
SW_Accum
2019-06-22
In which we play with fire
Used to subcalss and override certain services. 

In paticular the funky wind gust bug needs fixing.

"""



import syslog


def logmsg(level, msg):
    try:
        syslog.syslog(level, 'SW_Accum: %s' % msg)
    except TypeError as detail:
        # This happened onece on daemon stop. Well into a stop that seemed to be going well,
        #  a line was read in from serial and sent to loginf. Syslog spit out a TypeError
        #  and the program crashed and would not come back to life automatically.
        syslog.syslog(syslog.LOG_ERR, "SW_Accum: ERROR: syslog spit a TypeError exception")
        syslog.syslog(syslog.LOG_ERR, "SW_Accum: ERROR: Usually means string input has issues or is none")
        syslog.syslog(syslog.LOG_ERR, "SW_Accum: ERROR: TypeError: %s" % detail)
        syslog.syslog(syslog.LOG_ERR,"SW_Accum: Traceback: \n%s" % traceback.format_exc())
        syslog.syslog(syslog.LOG_INFO, "SW_Accum: Stoic will now try to put this behind it and continue with life")
    except:
        syslog.syslog(syslog.LOG_ERR, "SW_Accum: ERROR: syslog spit an error other than TypeError exception. (Raised to next level)")
        syslog.syslog(syslog.LOG_ERR,"SW_Accum: Traceback: \n%s" % traceback.format_exc())
        raise

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, 'ERROR: %s' % msg)

