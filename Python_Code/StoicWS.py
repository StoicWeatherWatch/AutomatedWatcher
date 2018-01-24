#!/usr/bin/env python
#
# Stoic WS
# Version 0.0.1
# 2018-01-24
#
# This is a driver for weeWX to connect with an Arduino based weather station.
# see
#http://www.weewx.com/docs/customizing.htm
#Under Implement the driver

"""weeWX Driver for Stoic Weather Watch.
An Arduino based weather station.

Stoic sends data constantly via a USB serial port. 

Data format
*Key,Value;

Supported Keys:          Mapped DB name
2T Temperature in Box  - extraTemp1

Alert format
!Key,Value;

All other line starts ignored, including #

Units used are weewx.METRICWX

CHIP from NEXTTHING sticks an arduino on a USB hub at ttyACM1
DEFAULT_PORT = "/dev/ttyACM1"

DEFAULT_BAUDRATE = 9600

"""

# TODO First readout after !Startup; is not reported. Used as baseline for wind etc.

#
#Device drivers should be written to emit None if a data value is bad 
#(perhaps because of a failed checksum). If the hardware simply doesn't 
#support it, then the driver should not emit a value at all.

# TODO DO I need this? Python 2.7 should have with_statement
from __future__ import with_statement

import serial
import syslog
import time
import binascii

import weewx.drivers

DRIVER_NAME = 'StoicWS'
DRIVER_VERSION = '0.0.1'

def loader(config_dict, _):
    return StoicWSDriver(**config_dict[DRIVER_NAME])

def confeditor_loader():
    return StoicWConfEditor()


def logmsg(level, msg):
    syslog.syslog(level, 'StoicWS: %s' % msg)

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, msg)
    



class StoicWSDriver(weewx.drivers.AbstractDevice):
    """weewx driver which receives data from a custom Arduino based weather station
    The Arduino sketch is strictly output. It constatly spits data.


    port - serial port
    [Required. Default is rather in question]

    max_tries - how often to retry serial communication before giving up
    [Optional. Default is 5]
    """


    def __init__(self, **stn_dict):
        self.model = stn_dict.get('model', 'StoicWS')
        # TODO Figure out the port
        self.port = stn_dict.get('port', StoicWatcher.DEFAULT_PORT)
        self.baudrate = stn_dict.get('baudrate', StoicWatcher.DEFAULT_BAUDRATE)
        self.max_tries = int(stn_dict.get('max_tries', 5))
        self.retry_wait = int(stn_dict.get('retry_wait', 3))
        debug_serial = int(stn_dict.get('debug_serial', 0))
        self.last_rain = None

        loginf('driver version is %s' % DRIVER_VERSION)
        loginf('using serial port %s' % self.port)

        self.StoicWatcher = StoicWatcher(self.port, self.baudrate, debug_serial=debug_serial)
        self.StoicWatcher.open()
        
    def closePort(self):
        if self.StoicWatcher is not None:
            self.StoicWatcher.close()
            self.StoicWatcher = None
        
    @property
    def hardware_name(self):
        return self.model
    
    # Stoic does not do time. Stoic is timeless. 
    def getTime(self):
        raise NotImplementedError("Method 'getTime' not implemented because Stoic does not do time")
    
    def setTime(self):
        raise NotImplementedError("Method 'setTime' not implemented because Stoic is timeless")
    
    def genLoopPackets(self):
        while True:
            packet = {'dateTime': int(time.time() + 0.5),
                      'usUnits': weewx.METRICWX}
            data = self.StoicWatcher.get_processed_data_with_retry(self.max_tries, self.retry_wait)
            packet.update(data)
            yield packet

class StoicWatcher(object):
    
    DEFAULT_PORT = "/dev/ttyACM1"
    DEFAULT_BAUDRATE = 9600
    
    def __init__(self, port, baudrate, debug_serial=0):
        self._debug_serial = debug_serial
        self.port = port
        self.baudrate = baudrate
        self.timeout = 3 # seconds
        self.serial_port = None
        
    def __enter__(self):
        self.open()
        return self

    def __exit__(self, _, value, traceback):
        self.close()
        
    def open(self):
        logdbg("open serial port %s" % self.port)
        self.serial_port = serial.Serial(self.port, self.baudrate,
                                         timeout=self.timeout)
        
        # python -m serial.tools.list_ports
        # Above will list ports


    def close(self):
        if self.serial_port is not None:
            logdbg("close serial port %s" % self.port)
            self.serial_port.close()
            self.serial_port = None
            
    def get_raw_data(self):
        LineIn = self.serial_port.readline()
        if self._debug_serial:
            logdbg("StoicWS said: %s" % LineIn)
            
        return LineIn
    
    def parse_2T_BoxTemp(self,LineIn):
        """
        Parse key 2T the temperature in the circuit box
        Should provide one value in 2 byte HEX eg
        1A5D
        
        entered as extraTemp1
        Units C
        """
        pos = LineIn.find(",")
        posEnd = LineIn.find(";")
        
        DataBin = int(LineIn[pos+1:posEnd],16)
        
        # Top three bits are garbage
        DataBin = DataBin & int("1FFF",16)
        
        #4th bit from highest is sign
        if((DataBin >> 12) == 0):
            Temp = float(DataBin & int("0FFF",16)) * (2**-4)
            
        else: # 1 and thus negative
            Temp = 256.0 - (float(DataBin & int("0FFF",16)) * (2**-4))
            
        data = dict()
        data["extraTemp1"] = Temp
        
        # TODO Remove line below. Testing only
        data["outTemp"] = Temp
        
        return data
        

        
        
    
    def parse_raw_data(self, LineIn):
        if LineIn[0] != "*":
            logdbg("StoicWS parse_raw_data given bad data")
            return None
        
        # Find the key
        pos = LineIn.find(",")
        
        # Call the correct parser based on the key
        #Temperature in the circuit box
        if LineIn[1:pos] == "2T":
            return parse_2T_BoxTemp(LineIn)

        else:
            return None
    
    @staticmethod
    def validate_string(LineIn):
        # TODO add aditional validation
        if (len(LineIn)<1):
            raise weewx.WeeWxIOError("Unexpected line length %d" % len(LineIn))
        if(LineIn[len(LineIn)-1] != ";"):
            raise weewx.WeeWxIOError("Line lacks terminator ; %s" % LineIn)
        #TODO uncomment this for proper validation
        #if(LineIn.find(",") == -1):
        #    raise weewx.WeeWxIOError("Line lacks seporator , %s" % LineIn)
        return LineIn
    
    
    def get_raw_data_with_retry(self, max_tries=5, retry_wait=3):
        for ntries in range(0, max_tries):
            try:
                LineIn = self.get_raw_data()
                self.validate_string(LineIn)
                return LineIn
            except (serial.serialutil.SerialException, weewx.WeeWxIOError), e:
                loginf("Failed attempt %d of %d to get readings: %s" %
                       (ntries + 1, max_tries, e))
                time.sleep(retry_wait)
        else:
            msg = "Max retries (%d) exceeded for readings" % max_tries
            logerr(msg)
            raise weewx.RetriesExceeded(msg)
        
    def get_processed_data_with_retry(self, max_tries=5, retry_wait=3):
        DataLine = False
        while not DataLine:
            LineIn = self.get_raw_data_with_retry(max_tries, retry_wait)
            if LineIn[0] == "*":
                # Data Line
                DataLine = True
                
            elif LineIn[0] == "!":
                # A computer readable alert
                # TODO Deal with computer readable alerts
                loginf('StoicWS: %s' % LineIn)
                
            
            elif LineIn[0] == "#":
                loginf('StoicWS: %s' % LineIn)
            #else:
                # Not a valid line, Ignored
        
        ParsedData = parse_raw_data(LineIn)
        
        if ParsedData == None:
            loginf('StoicWS: Unable to parse %s' % LineIn)
        
        return ParsedData
            
            
            
        




class StoicWConfEditor(weewx.drivers.AbstractConfEditor):
    @property
    def default_stanza(self):
        return 
    """
[StoicWS]
    # This section is for the Stoic Weather Watch station 

    # Serial port such as /dev/ttyS0, /dev/ttyUSB0, /dev/ttyACM1, or /dev/cua0 
    port = %s

    # The station model
    model = StoicWS

    # The driver to use:
    driver = weewx.drivers.StoicWS
""" % StoicWatcher.DEFAULT_PORT

   #TODO impliment other AbstractConfEditor functions

    def prompt_for_settings(self):
        print "Specify the serial port on which the station is connected."
        print "Typically /dev/ttyACM1 for Arduino connected to CHIP"
        port = self._prompt('port', Station.DEFAULT_PORT)
        return {'port': port}
    
    
    # define a main entry point for basic testing of the station without weewx
# engine and service overhead.  invoke this as follows from the weewx root dir:
#
# PYTHONPATH=bin python bin/weewx/drivers/ultimeter.py

if __name__ == '__main__':
    import optparse

    usage = """%prog [options] [--help]"""

    syslog.openlog('StoicWS', syslog.LOG_PID | syslog.LOG_CONS)
    syslog.setlogmask(syslog.LOG_UPTO(syslog.LOG_DEBUG))
    parser = optparse.OptionParser(usage=usage)
    parser.add_option('--version', dest='version', action='store_true',
                      help='display driver version')
    parser.add_option('--debug', dest='debug', action='store_true',
                      help='provide additional debug output in log')
    parser.add_option('--port', dest='port', metavar='PORT',
                      help='serial port to which the station is connected',
                      default=Station.DEFAULT_PORT)
    (options, args) = parser.parse_args()

    if options.version:
        print "StoicWS driver version %s" % DRIVER_VERSION
        exit(0)

    with StoicWatcher(options.port, debug_serial=options.debug) as StoicWatcher:
        while True:
            print time.time(), _fmt(StoicWatcher.get_raw_data())
    
