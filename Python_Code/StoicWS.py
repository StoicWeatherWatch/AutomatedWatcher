#!/usr/bin/env python
#
# Stoic WS
# Version 0.3.2
# 2019-06-22
#
# This is a driver for weeWX to connect with an Arduino and R Pi based weather station.
# see
#http://www.weewx.com/docs/customizing.htm
#Under Implement the driver

"""weeWX Driver for Stoic Weather Watch.
An Arduino based weather station with WeeWX running on a Raspberry Pi. 

Stoic Watcher sends data constantly via a USB serial port. 

Data format
*Key,Value(s);
Items on the same line
*Key,Value(s);+key,Value(s);

Supported Keys:                                    Mapped DB name
2T Temperature in Box  -                           extraTemp1
3TPH Temperature, pressure, humidity in FARS -     extraTemp2, pressure, outHumidity
   This is BME280ID "BME280-1"
4R Rain tipping bucket - rain
   Reported since last report - a difference
5WMD Wind direction mean
6WMS Wind speed mean
      6WMS and 5WMD are expected on the same line. (int
5WGD Wind gust direction. Reported as 4 readings which can be averaged.
6WGS wind speed gust. Reported as current, min, max. From 54 records. At 2.25 seconds 121.5 seconds
      6WGS and 5WGD are expected on the same line. 
7T fars temp sensor                                outTemp
2XT 1 Wire Temp Sensors  x {0..7}


Alert format
!Key,Value;

All other line starts ignored, including #

Units used are weewx.METRICWX

Raspberry Pi sticks an arduino on a USB hub at ttyACM1
DEFAULT_PORT = "/dev/ttyACM0"

DEFAULT_BAUDRATE = 9600

Looks for the following in the config file
rain_mm_Per_Tip - Tipping bucket conversion factor 0.2794

STOIC Stoic Thing Observes Information on Climate 

"""




# TODO If your lose the  connection, you should raise an exception of type 
# weewx.WeeWxIOError, or a subclass of weewx.WeeWxIOError. The engine will catch this exception 
# and, by default, after 60 seconds do a restart. This will cause your driver to reload, giving it an 
# opportunity to reconnect with the broker.


#Device drivers should be written to emit None if a data value is bad 
#(perhaps because of a failed checksum). If the hardware simply doesn't 
#support it, then the driver should not emit a value at all.

# TODO crashes when Aruino dissconnected. Handle gracefully


# TODO vcgencmd measure_temp for R Pi
# TODO add chip temp for Arduino

# TODO crashes on disconnect of I2C sensor both the CHIP and probabily the arduino



# TODO DO I need this? Python 2.7 should have with_statement
from __future__ import with_statement

import serial
import syslog
import time
import traceback
import serial.tools.list_ports

from SW_RemoteWatcher import SW_RemoteWatcher

# TODO do I use binascii?
import binascii

import weewx.drivers

DRIVER_NAME = 'StoicWS'
DRIVER_VERSION = '0.3.2'

def loader(config_dict, _):
    return StoicWSDriver(**config_dict[DRIVER_NAME])

def confeditor_loader():
    return StoicWConfEditor()

#LOCAL_LOG_FILE = "/home/pi/Stoic_LOG.txt"

def logmsg(level, msg):
    try:
	syslog.syslog(level, 'StoicWS: %s' % msg)
    except TypeError as detail:
	# This happened onece on daemon stop. Well into a stop that seemed to be going well,
	#  a line was read in from serial and sent to loginf. Syslog spit out a TypeError
	#  and the program crashed and would not come back to life automatically.
	syslog.syslog(syslog.LOG_ERR, "StoicWS: ERROR: syslog spit a TypeError exception")
	syslog.syslog(syslog.LOG_ERR, "StoicWS: ERROR: Usually means string input has issues or is none")
	syslog.syslog(syslog.LOG_ERR, "StoicWS: ERROR: TypeError: %s" % detail)
	syslog.syslog(syslog.LOG_ERR,"StoicWS: Traceback: \n%s" % traceback.format_exc())
	syslog.syslog(syslog.LOG_INFO, "StoicWS: Stoic will now try to put this behind it and continue with life")
    except:
	syslog.syslog(syslog.LOG_ERR, "StoicWS: ERROR: syslog spit an error other than TypeError exception. (Raised to next level)")
	syslog.syslog(syslog.LOG_ERR,"StoicWS: Traceback: \n%s" % traceback.format_exc())
	raise

def logdbg(msg):
    logmsg(syslog.LOG_DEBUG, msg)

def loginf(msg):
    logmsg(syslog.LOG_INFO, msg)

def logerr(msg):
    logmsg(syslog.LOG_ERR, 'ERROR: %s' % msg)
    
# TODO replace a lot of the loginf with logdbg


class StoicWSDriver(weewx.drivers.AbstractDevice):
    """weewx driver which receives data from a custom Arduino based weather station
    The Arduino sketch is strictly output. It constatly spits data.


    port - serial port
    [Required. Default is rather in question]

    max_tries - how often to retry serial communication before giving up
    [Optional. Default is 5]
    """
    
        
    
    def readCalibrationDict(self, stn_dict):
        
        def BoschHEXHEX2UnsignedLong(msb,lsb):

            return  ((long(msb,16) << 8) + long(lsb,16))
            
        def BoschHEXHEX2SignedLong(msb,lsb):
            
            if((long(msb,16) >> 7) == 1):
                sign = long(-1)
            else:
                sign = long(1)

            return (sign * (((long(msb,16) & 0b01111111) << 8) + long(lsb,16)))
        
        stoic_Cal_dict = dict()
         
        
        loginf("StoicWSDriver.readCalibrationDict running")
         
        # TODO make this a try so it fails gracefully when not in the dictionary
        
        #TODO Handle a value not present gracefully
        #try:
        
        # General
        # Serial
        stoic_Cal_dict["serial_port_default"] = stn_dict.get('serial_port_default')
        stoic_Cal_dict["serial_port_Prefix"] = stn_dict.get('serial_port_Prefix')
        stoic_Cal_dict["serial_port_Lowest"] = int(stn_dict.get('serial_port_Lowest'))
        
        stoic_Cal_dict["round_values_down_to"] = int(stn_dict.get('round_values_down_to'))
        
        # Rain
        stoic_Cal_dict["rain_mm_Per_Tip"] = float(stn_dict.get('rain_mm_Per_Tip'))
        
        # Wind Speed
        stoic_Cal_dict["wind_mps_per_click_per_2_25_s"] = float(stn_dict.get('wind_mps_per_click_per_2_25_s'))
        stoic_Cal_dict["wind_mps_per_click_per_120_s"] = float(stn_dict.get('wind_mps_per_click_per_120_s'))
        
        # Wind Direction
        stoic_Cal_dict["wind_direction_cal_Main_Model_offset"] = float(stn_dict.get('wind_direction_cal_Main_Model_offset'))
        stoic_Cal_dict["wind_direction_cal_Main_Model_slope"] = float(stn_dict.get('wind_direction_cal_Main_Model_slope'))
        stoic_Cal_dict["wind_direction_cal_Low_Model_offset"] = float(stn_dict.get('wind_direction_cal_Low_Model_offset'))
        stoic_Cal_dict["wind_direction_cal_Low_Model_slope"] = float(stn_dict.get('wind_direction_cal_Low_Model_slope'))
        stoic_Cal_dict["wind_direction_cal_High_Model_offset"] = float(stn_dict.get('wind_direction_cal_High_Model_offset'))
        stoic_Cal_dict["wind_direction_cal_High_Model_slope"] = float(stn_dict.get('wind_direction_cal_High_Model_slope'))
        stoic_Cal_dict["wind_direction_Low_Model_Cutoff"] = int(stn_dict.get('wind_direction_Low_Model_Cutoff'))
        stoic_Cal_dict["wind_direction_High_Model_Cutoff"] = int(stn_dict.get('wind_direction_High_Model_Cutoff'))
        
        
        stoic_Cal_dict["wind_direction_dead_zone_dir"] = float(stn_dict.get('wind_direction_dead_zone_dir'))
        
        # Wind constants for mean wind direction 
        stoic_Cal_dict["wind_direction_number_of_mean_points"] = int(stn_dict.get('wind_direction_number_of_mean_points'))
        stoic_Cal_dict["wind_direction_number_of_bins"] = int(stn_dict.get('wind_direction_number_of_bins'))
        stoic_Cal_dict["wind_direction_half_bin_size"] = int(stn_dict.get('wind_direction_half_bin_size'))
        stoic_Cal_dict["wind_direction_max_ADC"] = int(stn_dict.get('wind_direction_max_ADC'))

        #  Data sheet T1 unsigned 
        stoic_Cal_dict["BME280_1_CAL_T1"] = BoschHEXHEX2UnsignedLong(stn_dict.get("cal-BME280-1.2.1"),stn_dict.get("cal-BME280-1.2.0"))
        #  Data sheet T2 signed 
        stoic_Cal_dict["BME280_1_CAL_T2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.3"),stn_dict.get("cal-BME280-1.2.2"))
        #  Data sheet T3 Signed
        stoic_Cal_dict["BME280_1_CAL_T3"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.5"),stn_dict.get("cal-BME280-1.2.4"))
        #  Data sheet P1 Unsigned
        stoic_Cal_dict["BME280_1_CAL_P1"] = BoschHEXHEX2UnsignedLong(stn_dict.get("cal-BME280-1.2.7"),stn_dict.get("cal-BME280-1.2.6"))
        #  Data sheet P2 signed
        stoic_Cal_dict["BME280_1_CAL_P2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.9"),stn_dict.get("cal-BME280-1.2.8"))
        #  Data sheet P3 signed
        stoic_Cal_dict["BME280_1_CAL_P3"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.11"),stn_dict.get("cal-BME280-1.2.10"))
        #  Data sheet P4 signed
        stoic_Cal_dict["BME280_1_CAL_P4"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.13"),stn_dict.get("cal-BME280-1.2.12"))
        #  Data sheet P5 signed
        stoic_Cal_dict["BME280_1_CAL_P5"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.15"),stn_dict.get("cal-BME280-1.2.14"))
        #  Data sheet P6 signed
        stoic_Cal_dict["BME280_1_CAL_P6"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.17"),stn_dict.get("cal-BME280-1.2.16"))
        #  Data sheet P7 signed
        stoic_Cal_dict["BME280_1_CAL_P7"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.19"),stn_dict.get("cal-BME280-1.2.18"))
        #  Data sheet P8 signed 
        stoic_Cal_dict["BME280_1_CAL_P8"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.21"),stn_dict.get("cal-BME280-1.2.20"))
        #  Data sheet P9 signed
        stoic_Cal_dict["BME280_1_CAL_P9"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.23"),stn_dict.get("cal-BME280-1.2.22"))
        #  Data sheet H1 unsigned single byte memory A1
        #   A0 is skipped
        stoic_Cal_dict["BME280_1_CAL_H1"] = BoschHEXHEX2UnsignedLong("00",stn_dict.get("cal-BME280-1.2.25"))
        #  Data sheet H2 signed memory E1 and E2
        stoic_Cal_dict["BME280_1_CAL_H2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.1.1"),stn_dict.get("cal-BME280-1.1.0"))
        #  Data sheet H3 unsigned single byte memory E3
        stoic_Cal_dict["BME280_1_CAL_H3"] = BoschHEXHEX2UnsignedLong("00",stn_dict.get("cal-BME280-1.1.2"))
        #  Data sheet H4 signed 12 bits. E4 holds the most significant 8 and the least significant 4 are the low 4 of E5
        sign = -1 if (long(stn_dict.get("cal-BME280-1.1.3"),16) >> 7 == 1) else 1
        stoic_Cal_dict["BME280_1_CAL_H4"] = (long(sign) * (((long(stn_dict.get("cal-BME280-1.1.3"),16) & 0b01111111) << 4) + (long(stn_dict.get("cal-BME280-1.1.4"),16) & 0b00001111) ))
        #  Data sheet H5 signed 12 bits. E5 holds the least significant 4 bits in its high 4 and E6 holds the most significant bits
        sign = -1 if (long(stn_dict.get("cal-BME280-1.1.5"),16) >> 7 == 1) else 1
        stoic_Cal_dict["BME280_1_CAL_H5"] = (long(sign) * (((long(stn_dict.get("cal-BME280-1.1.5"),16) & 0b01111111) << 4) + (long(stn_dict.get("cal-BME280-1.1.4"),16) >> 4) ))
       
        
        #  Data sheet H6 signed byte E7
        sign = -1 if (long(stn_dict.get("cal-BME280-1.1.6"),16) >> 7 == 1) else 1
        stoic_Cal_dict["BME280_1_CAL_H6"] = long(sign) * (long(stn_dict.get("cal-BME280-1.1.6"),16) & 0b01111111)
        
        # BMP280 in PRS
        stoic_Cal_dict["BMP280_2_CAL_T1"] = BoschHEXHEX2UnsignedLong(stn_dict.get("cal-BMP280-2.2.1"),stn_dict.get("cal-BMP280-2.2.0"))
        #  Data sheet T2 signed 
        stoic_Cal_dict["BMP280_2_CAL_T2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.3"),stn_dict.get("cal-BMP280-2.2.2"))
        #  Data sheet T3 Signed
        stoic_Cal_dict["BMP280_2_CAL_T3"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.5"),stn_dict.get("cal-BMP280-2.2.4"))
        #  Data sheet P1 Unsigned
        stoic_Cal_dict["BMP280_2_CAL_P1"] = BoschHEXHEX2UnsignedLong(stn_dict.get("cal-BMP280-2.2.7"),stn_dict.get("cal-BMP280-2.2.6"))
        #  Data sheet P2 signed
        stoic_Cal_dict["BMP280_2_CAL_P2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.9"),stn_dict.get("cal-BMP280-2.2.8"))
        #  Data sheet P3 signed
        stoic_Cal_dict["BMP280_2_CAL_P3"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.11"),stn_dict.get("cal-BMP280-2.2.10"))
        #  Data sheet P4 signed
        stoic_Cal_dict["BMP280_2_CAL_P4"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.13"),stn_dict.get("cal-BMP280-2.2.12"))
        #  Data sheet P5 signed
        stoic_Cal_dict["BMP280_2_CAL_P5"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.15"),stn_dict.get("cal-BMP280-2.2.14"))
        #  Data sheet P6 signed
        stoic_Cal_dict["BMP280_2_CAL_P6"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.17"),stn_dict.get("cal-BMP280-2.2.16"))
        #  Data sheet P7 signed
        stoic_Cal_dict["BMP280_2_CAL_P7"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.19"),stn_dict.get("cal-BMP280-2.2.18"))
        #  Data sheet P8 signed 
        stoic_Cal_dict["BMP280_2_CAL_P8"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.21"),stn_dict.get("cal-BMP280-2.2.20"))
        #  Data sheet P9 signed
        stoic_Cal_dict["BMP280_2_CAL_P9"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BMP280-2.2.23"),stn_dict.get("cal-BMP280-2.2.22"))
        
        stoic_Cal_dict["Temp1W-schema-0"] = stn_dict.get('Temp1W-schema-0')
        stoic_Cal_dict["Temp1W-schema-1"] = stn_dict.get('Temp1W-schema-1')
        stoic_Cal_dict["Temp1W-schema-2"] = stn_dict.get('Temp1W-schema-2')
        stoic_Cal_dict["Temp1W-schema-3"] = stn_dict.get('Temp1W-schema-3')
        stoic_Cal_dict["Temp1W-schema-4"] = stn_dict.get('Temp1W-schema-4')
        stoic_Cal_dict["Temp1W-schema-5"] = stn_dict.get('Temp1W-schema-5')
        stoic_Cal_dict["Temp1W-schema-6"] = stn_dict.get('Temp1W-schema-6')
        stoic_Cal_dict["Temp1W-schema-7"] = stn_dict.get('Temp1W-schema-7')
 

        
        # Test stuff
#         for i in range(1,3+1):
#              logdbg("stoic_Cal_dict['BME280_1_CAL_T" + str(i) +"  0x%X" % stoic_Cal_dict["BME280_1_CAL_T"+str(i)])
#         for i in range(1,9+1):
#              logdbg("stoic_Cal_dict['BME280_1_CAL_P" + str(i) +"  0x%X" % stoic_Cal_dict["BME280_1_CAL_P"+str(i)])
#         for i in range(1,6+1):
#              logdbg("stoic_Cal_dict['BME280_1_CAL_H" + str(i) +"  0x%X" % stoic_Cal_dict["BME280_1_CAL_H"+str(i)])

	loginf("StoicWSDriver.readCalibrationDict  Done")
        
        return stoic_Cal_dict
    
    #**Variable inside a function signiture (as below), means take any aditional arguments and
    # cram them into the dictionary as key value pairs. Thus here we get the dictionary from weewx.conf
    # and anything else weewx cares to send.
    def __init__(self, **stn_dict):
        
        loginf("StoicWSDriver version %s init" %DRIVER_VERSION)
        self.model = stn_dict.get('model', 'StoicWS')
        # TODO Figure out the port
        self.port = stn_dict.get('serial_port_default', StoicWatcher.DEFAULT_PORT)
        self.baudrate = stn_dict.get('baudrate', StoicWatcher.DEFAULT_BAUDRATE)
        self.max_tries = int(stn_dict.get('max_tries', 25))
        self.maxTrysBeforeCloseAndOpenPort = int(stn_dict.get('max_tries_before_cycle_port', 25))
        self.retry_wait = int(stn_dict.get('retry_wait', 3))
        debug_serial = int(stn_dict.get('debug_serial', 0))
	
        
        #No need for this. The hardware resets the rain count when the serial port is reset.
        # Also the hardware reports its own last rain
        self.last_rain = None

        logdbg('driver version is %s' % DRIVER_VERSION)
        loginf('using serial port %s' % self.port)
	logdbg('debug_serial is %d' % debug_serial)
        
        
        stoic_Cal_dict = self.readCalibrationDict(stn_dict)

	try:
            self.StoicWatcher = StoicWatcher(self.port, self.baudrate, stoic_Cal_dict, debug_serial, stn_dict)
            #self.StoicWatcher = StoicWatcher(self.port, self.baudrate, debug_serial=debug_serial)
	except TypeError as detail:
	    logerr("StoicWSDriver create StoicWatcher failed with TypeError")
	    logerr(detail)
	    logerr("Traceback: \n%s" % traceback.format_exc())
	    raise
	except:
	    logerr("StoicWSDriver create StoicWatcher failed with something other than TypeError")
	    logerr("Traceback: \n%s" % traceback.format_exc())
	    raise

	loginf("StoicWSDriver __init__  Calling self.StoicWatcher.open()")
        self.StoicWatcher.open()
        
        #This is for receiving form remote sensors
        #TODO Set the port from conf file
        loginf("StoicWSDriver opening SW_RemoteWatcher")
        self.RemoteSource = SW_RemoteWatcher(1216,dict())
        loginf("StoicWSDriver init done")
        self.RemoteSource.StartMonitoringForRemotes()
       
    # WeeWX calls this when it wants to terminate. 
    def closePort(self):
        loginf("StoicWSDriver closePort called") 
        if self.StoicWatcher is not None:
            self.StoicWatcher.close()
            self.StoicWatcher = None
        
	loginf("StoicWSDriver closePort closing RemoteSource") 
        if self.RemoteSource is not None:
            self.RemoteSource.StopMonitoringForRemotes()
            self.RemoteSource = None
		
	loginf("StoicWSDriver closePort done") 
        
    @property
    def hardware_name(self):
        return self.model
    
    # Stoic does not do time. Stoic is timeless. 
    def getTime(self):
        raise NotImplementedError("Method 'getTime' not implemented because Stoic does not do time")
    
    def setTime(self):
        raise NotImplementedError("Method 'setTime' not implemented because Stoic is timeless")
    
    def genLoopPackets(self):
        """
        This is where the action is
        """
        loginf("StoicWSDriver genLoopPackets")
        
        while True:
            packet = {'dateTime': int(time.time() + 0.5),
                      'usUnits': weewx.METRICWX}
            data = self.StoicWatcher.get_processed_data_with_retry(self.max_tries, self.maxTrysBeforeCloseAndOpenPort, self.retry_wait)
            
            if data != None:
                packet.update(data)
                yield packet
            
            # This handles the remote sensors
            packet2 = {'dateTime': int(time.time() + 0.5),'usUnits': weewx.METRICWX}
            
            data2 = self.RemoteSource.GetData()
            
            
            if data2 != None:
                packet2.update(data2)
                yield packet2

class StoicWatcher(object):
    
    DEFAULT_PORT = "/dev/ttyACM0"
    DEFAULT_BAUDRATE = 9600
    
    _ListOfDisabledSensors = ["24T",
                              "26T",
                              "27T",
                              "28T"]
    
    #**Variable inside a function signiture (as below), means take any aditional arguments and
    # cram them into the dictionary as key value pairs. Which is great except it cannot handle a 
    # dictionary comming through. Oddly so **stoic_Sation_dict gives type exception too many args.
    def __init__(self, port, baudrate, stoic_Cal_dict, debug_serial, stoic_Station_dict):
	
	loginf("StoicWS StoicWatcher __init__  Starting")

    #def __init__(self, port, baudrate, debug_serial=0):
        self._debug_serial = debug_serial
        self.port = port
        self.baudrate = baudrate
        self.timeout = 3 # seconds
        self.serial_port = None
        
        self.stoic_Cal_dict = stoic_Cal_dict
	
	self.local_log_file = stoic_Station_dict.get('local_log_file', None)
	self.save_bang_Ard = int(stoic_Station_dict.get('save_bang', 0))
	logdbg('StoicWatcher __init__ save_bang_Ard = %d' % self.save_bang_Ard)
	logdbg('StoicWatcher __init__ local_log_file = %s' % self.local_log_file)
	
	# Not currently used anywhere. May be best to process it here anyway
	#self.stoic_Sation_dict = stoic_Sation_dict
	

        
    def __enter__(self):
        self.open()
        return self

    def __exit__(self, _, value, traceback):
        self.close()
        
    def open(self):
        
        # TODO validate that we are talking to the Arduino and not something else
        
        if len(list(serial.tools.list_ports.comports())) == 0:
            loginf("No serial ports found")
        
        NumTrys = 17
        PortNum = self.stoic_Cal_dict["serial_port_Lowest"]
        for i in range(NumTrys):
            try:
                loginf("Attempting to Open com channel %s" % self.port)
                self.serial_port = serial.Serial(self.port, self.baudrate,timeout=self.timeout)
            except (serial.serialutil.SerialException) as e:
                loginf("Failed to open serial port: %s" % e)
                logdbg(traceback.format_exc())
                
                # Sometimes the arduino comes in on a different port. 
                if str(e).find("could not open port") != -1:
                    if str(e).find("No such file or directory") != -1:
                        loginf("Attempting a different port")
                        
                        self.port = self.stoic_Cal_dict["serial_port_Prefix"] + str(PortNum)
                        PortNum += 1
                
                # on the last try give up completely, on the others take a nap
                if i == NumTrys - 1:
                    raise
                else:
                    time.sleep(5)
            else:
                break
            
            # TODO Handle it when it trys to talk to somethign that is not an Arduino
        
        # python -m serial.tools.list_ports
        # Above will list ports

# TODO should we handle rain in some way here
    def close(self):
        loginf("Close com channel >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" )
        if self.serial_port is not None:
            logdbg("close serial port %s" % self.port)
            self.serial_port.close()
            self.serial_port = None
	logdbg("StoicWatcher close Done" )
            
    def get_raw_data(self):
        LineIn = self.serial_port.readline()
        
        # _debug_serial is set in [StoicWS] of weewx.conf
        if self._debug_serial:
            logdbg("LineIn %s" % LineIn)
            
        return LineIn
    
    def trim_Data_Reasonable_Places(self,valueIn):
        return round(valueIn, self.stoic_Cal_dict["round_values_down_to"])
        
    
    def result_check_temp(self,Temperature):
        """
        Sets limits on Temperature. C is assumed
        
        This should not be used on diagnostic sensors in boxes and containers since it may kill real values.
        """
        
        if Temperature > 50:
            return False
        if Temperature < -25:
            return False
        
        return True
    
    def sensor_parse_MCP9808_Temp(self, DataHex):
        """
        Parse the two hex bytes output from an MCP9808 tempriture sensor.
        Returns tempriture C
        The MCP9808 outputs 13 bit data in twos complement. (Sort of twos compliment.)
        The data is fixed point. 
        """
        DataRaw = int(DataHex,16)
        
        # Top three bits are garbage
        DataRaw = DataRaw & int("1FFF",16)
        
        #4th bit from highest is sign. Positive if zero
        if((DataRaw >> 12) == 0):
            Temp = float(DataRaw & int("0FFF",16)) * (2**-4)
            
        else: # 1 and thus negative
            Temp = (float(DataRaw & int("0FFF",16)) * (2**-4)) - 256.0
            
        return Temp
            
            
        
    
    def key_parse_2T_BoxTemp(self,LineIn):
        """
        Parse key 2T the temperature in the circuit box
        Should provide one value in 2 byte HEX eg
        1A5D
        
        entered as extraTempBox
        Units C
        Sesnor is an MCP9808 on the I2C bus
        Input format *2T,xxxx; or *2T,xxxx,^;  xxxx is two hex bytes read from the sensor.
        """
        # TODO add ! on arduino side if not detected. On this side respond by not recording data. 
        # TODO add check of sensors to 90 min house keeping?
        
        # Pull the data from the text string that came in.
        posStart = LineIn.find(",")
        # Does the line have a checksum indicator
        if(LineIn.find("^") == -1):
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn[posStart+1:].find(",") + posStart + 1
            # TODO never tested with ^ 
        
        Temp = self.sensor_parse_MCP9808_Temp(LineIn[posStart+1:posEnd])
        
        logdbg("key_parse_2T_BoxTemp temp %f" % Temp)
        
        Temp = self.trim_Data_Reasonable_Places(Temp)
            
        data = dict()
        data["extraTempBox"] = Temp
        
        return data
    
    
    
    def key_parse_7T_FARSTemp(self, LineIn):
        """
        Parse key 7T the temperature in the FARS
        Should provide one value in 2 byte HEX eg
        1A5D
        
        entered as outTemp
        Units C
        Sensor is an MCP9808 on the I2C bus
        Input format *7T,xxxx; or *7T,xxxx,^;  xxxx is two hex bytes read from the sensor.
        """
        # TODO add ! on arduino side if not detected. On this side respond by not recording data. 
        # TODO add check of sensors to 90 min house keeping?
        
        # Pull the data from the text string that came in.
        posStart = LineIn.find(",")
        # Does the line have a checksum indicator
        if(LineIn.find("^") == -1):
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn[posStart+1:].find(",") + posStart + 1
            # TODO never tested with ^ 
        
        Temp = self.sensor_parse_MCP9808_Temp(LineIn[posStart+1:posEnd])
        
        Temp = self.trim_Data_Reasonable_Places(Temp)
        
        logdbg("key_parse_7T_FARSTemp temp %f" % Temp)
            
        data = dict()
        data["TempFARS"] = Temp
        
        return data      
    
    
    
    def sensor_parse_BME280_Pressure(self, DataHex, BME280ID, TFine):
        """
        The BME280ID allows for mutiple BME280s on a single system with seporate calibrations.
        
        Pressure arrives as 3 hex bytes.
        Byte struction MSB xxxx xxxx LSB xxxx xxxx XLSB xxxx 0000
        
        Output in units of hPa (Same as mbar)
        Near as I can tell from the data sheet, the output is pressure. Not compensented for altitude. (How could it be? 
        The sensor does not know its altitude)
        """
        
        #logdbg("Stoic sensor_parse_BME280_Pressure  HEX in %s" % DataHex)
        
        RawPressure = long(DataHex,16) >> 4
        
        #logdbg("Stoic sensor_parse_BME280_Pressure  RawPressure %d" % RawPressure)
        #logdbg("Stoic sensor_parse_BME280_Pressure  RawPressure %X" % RawPressure)
        
        
        # TEST Line
        #logdbg("self.stoic_Cal_dict[BME280ID+'_CAL_P6''] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_P6"])
        #logdbg("self.stoic_Cal_dict[BME280ID+'_CAL_P6''] type  %s" % type(self.stoic_Cal_dict[BME280ID+"_CAL_P6"]))

        #var1 = ((BME280_S64_t)t_fine) - 128000;
        var1 = TFine - long(128000)
        
        #var2 = var1 * var1 * (BME280_S64_t)dig_P6;
        var2 = var1 * var1 * self.stoic_Cal_dict[BME280ID+"_CAL_P6"]
        
        #var2 = var2 + ((var1*(BME280_S64_t)dig_P5)<<17);
        var2 = var2 + ( (var1*self.stoic_Cal_dict[BME280ID+"_CAL_P5"]) << 17)
        
        #var2 = var2 + (((BME280_S64_t)dig_P4)<<35);
        var2 = var2 + ((self.stoic_Cal_dict[BME280ID+"_CAL_P4"]) << 35)
        
        #var1 = ((var1 * var1 * (BME280_S64_t)dig_P3)>>8) + ((var1 * (BME280_S64_t)dig_P2)<<12);
        var1 = ((var1 * var1 * self.stoic_Cal_dict[BME280ID+"_CAL_P3"]) >> 8) + ((var1 * self.stoic_Cal_dict[BME280ID+"_CAL_P2"]) << 12)
        
        #var1 = (((((BME280_S64_t)1)<<47)+var1))*((BME280_S64_t)dig_P1)>>33;
        var1 = ( ( (long(1)<<47) + var1 ) * (self.stoic_Cal_dict[BME280ID+"_CAL_P1"]) ) >> 33
        
        #if (var1 == 0) {return 0; // avoid exception caused by division by zero}
        # This avoides a division by zero
        if(var1 == 0):
            return 0
        # TODO When will this actually be zero. Should it return none?
        
        #p = 1048576-adc_P;
        p = long(1048576) - RawPressure
        
        
        #p = (((p<<31)-var2)*3125)/var1;
        p = ( ((p<<31)-var2 ) * long(3125) ) / var1
        
        
        #var1 = (((BME280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
        var1 = ((self.stoic_Cal_dict[BME280ID+"_CAL_P9"]) * (p>>13) * (p>>13)) >> 25
        
        #var2 = (((BME280_S64_t)dig_P8) * p) >> 19;
        var2 = ((self.stoic_Cal_dict[BME280ID+"_CAL_P9"]) * p) >> 19
        
        #p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7)<<4);
        p = ((p + var1 + var2) >> 8) + (self.stoic_Cal_dict[BME280ID+"_CAL_P7"] << 4)
        
    # p is integer pressure. p / 256  gives Pa.  (p / 256) / 100 gives hPa
        #logdbg("Stoic sensor_parse_BME280_Pressure  Integer Pressure %d" % p)
        #logdbg("Stoic sensor_parse_BME280_Pressure  Integer Pressure %X" % p)
        
        # Units of hPa or mbar (same thing)
        Pressure = float(p) / float(25600.0)

        #logdbg("Stoic sensor_parse_BME280_Pressure Pressure %f" % Pressure)

        return Pressure
        
            
    def sensor_parse_BME280_TFine(self, DataHex, BME280ID):
        """
        The BME280ID allows for mutiple BME280s on a single system
        
        The BME280 calibration produces a value called TFine which is used in later calibrations. 
        TFine is returned by this function.
        
        Input is 3 byte of HEX. Only 20 bits have meaning
        """
    
        
        RawTemp = long(DataHex,16) >> 4
        
        
        # This mess comes from the data sheet. Someone must like LISP
        var1  = ( ((RawTemp>>3) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"]<<1)) * (self.stoic_Cal_dict[BME280ID+"_CAL_T2"]) ) >> 11

        var2  = (( ( ((RawTemp>>4) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"])) * ((RawTemp>>4) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"])) ) >> 12) * (self.stoic_Cal_dict[BME280ID+"_CAL_T3"]) ) >> 14

        TFine = var1 + var2

        # The limits for the sensor are -40 to 85 C. Equivalent to an output of -4000 or 8500
        if (TFine < -204826):
            loginf("Stoic sensor_parse_BME280_TFine T less than min. Reporting None")
            return None
        elif (TFine > 435174):
            loginf("Stoic sensor_parse_BME280_TFine T greater than max. Reporting None")
            return None

        return TFine
    
    def sensor_parse_BME280_Temperature(self, TFine):
        """
        Takes in TFine and output tempreature in C
        """
        if TFine == None:
            return None
        
        Temperature = float((TFine * 5 ) >> 8)/float(100.0)
        
        return Temperature
            
    def sensor_parse_BME280_Humidity(self, DataHex, BME280ID, TFine):
        """
        The BME280ID allows for mutiple BME280s on a single system
        
        Formulae are based on BME280 Data sheet. 
        Output is realtive humidity as a percentage
        """
        
        RawHum = long(DataHex,16)
   
        
        # // Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits). 
        #// Output value of 47445 represents 47445/1024 = 46.333 %RH 
   
        #    var1 = (TFine - ((BME280_S32_t)76800));
        var1 = TFine - long(76800)
        
       # var1 = (((((adc_H << 14) - (((BME280_S32_t)dig_H4) << 20) - (((BME280_S32_t)dig_H5) * var1)) + ((BME280_S32_t)16384)) >> 15)
        #* ((( ((((var1 * ((BME280_S32_t)dig_H6)) >> 10) * (((var1 * ((BME280_S32_t)dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) 
       #        + ((BME280_S32_t)2097152)) * ((BME280_S32_t)dig_H2) + 8192) >> 14));  
        var1 = ( ((((RawHum << 14) 
                   - (self.stoic_Cal_dict[BME280ID+"_CAL_H4"] << 20) 
                   - (self.stoic_Cal_dict[BME280ID+"_CAL_H5"] * var1)) 
                   + long(16384)) >> 15) 
                   * ((( ( (( (var1 * self.stoic_Cal_dict[BME280ID+"_CAL_H6"]) >> 10) 
                            * (((var1 * (self.stoic_Cal_dict[BME280ID+"_CAL_H3"])) >> 11) + long(32768))) >> 10) 
                            + long(2097152)) * self.stoic_Cal_dict[BME280ID+"_CAL_H2"]
                            + long(8192)) >> 14) )
        
        
        #var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((BME280_S32_t)dig_H1)) >> 4));
        var1 = (var1 - ( ( (((var1 >> 15) * (var1 >> 15)) >> 7) * self.stoic_Cal_dict[BME280ID+"_CAL_H1"] ) >> 4 ))
        
        #var1 = (var1 < 0 ? 0 : var1);   
        # If ? Then : else
        if var1 < 0:
            var1 = 0
        # Else no change
        
        #var1 = (var1 > 419430400 ? 419430400 : var1);   
        if var1 > long(419430400):
            var1 = long(419430400)
        # Else no change
        
        H = var1 >> 12
        
        
        # Hum = float(H) / float(1024)
        # Because we like complexity.       12345678901234567890121234567890
        Hum = float(H >> 10) + (float(H & 0b00000000000000000000001111111111) / float(1024))
        

        return Hum
    
    def BME280_line_validation(self,LineIn):
        """
        *3TPH,PPPPPP,TTTTTT,HHHH,^;
        *3TPH,5EC2E0,7E40D0,69A1,^;
        
        For now simply test for all zeros
        
        """
        
        if LineIn.find("000000,000000,0000") != -1:
            loginf("BME280_line_validation failed: %s" %LineIn)
            return False

        return True
    
    
    def key_parse_3TPH_FARS(self,LineIn):
        """
        Data from BME280
        Line in follows 
        *3TPH,PPPPPP,TTTTTT,HHHH,^(Check bits may go here some day);
        *3TPH,5EC2E0,7E40D0,69A1,^;
        """
        
        BME280ID = "BME280_1"
        
        if not self.BME280_line_validation(LineIn):
            return None
        
        
        # Pressure
        PposStart = LineIn.find(",")
        PposEnd = PposStart + LineIn[PposStart+1:].find(",")
            
        # Temperature
        TposStart = PposEnd+1
        TposEnd = TposStart + LineIn[TposStart+1:].find(",")
        
        # Humidity
        HposStart = TposEnd+1
        
        # Check sums are inserted as ,^cksum; otherwise data ends with ;
        if(LineIn.find("^") == -1):
            HposEnd = HposStart + LineIn[HposStart+1:].find(";")
        else:
            HposEnd = HposStart + LineIn[HposStart+1:].find(",")
            
        TFine = self.sensor_parse_BME280_TFine(LineIn[TposStart+1:TposEnd+1],BME280ID)
        
        if(TFine == None):
            data = dict()
            data["extraTempFARS"] = None
            data["pressureFARS"] = None
            data["HumidityFARS"] = None
            return data
        
        Temperature = self.sensor_parse_BME280_Temperature(TFine)
        
        Pressure = self.sensor_parse_BME280_Pressure(LineIn[PposStart+1:PposEnd+1],BME280ID, TFine)
            
        Humidity = self.sensor_parse_BME280_Humidity(LineIn[HposStart+1:HposEnd+1],BME280ID, TFine)
        
        Temperature = self.trim_Data_Reasonable_Places(Temperature)
        Pressure = self.trim_Data_Reasonable_Places(Pressure)
        Humidity = self.trim_Data_Reasonable_Places(Humidity)
        
        logdbg("key_parse_3TPH_FARS Temperature %f" % Temperature)
        logdbg("key_parse_3TPH_FARS Pressure %f" % Pressure)
        logdbg("key_parse_3TPH_FARS Humidity %f" % Humidity)
        
        data = dict()
        data["extraTempFARS"] = Temperature
        data["pressureFARS"] = Pressure # FARS is causing issues
        data["HumidityFARS"] = Humidity
        return data
    
    def BMP280_line_validation(self,LineIn):
        """
        *9TP,PPPPPP,TTTTTT,^;
        *9TP,5EC2E0,7E40D0,^;
        
        For now simply test for all zeros
        
        """
        
        if LineIn.find("000000,000000") != -1:
            loginf("BMP280_line_validation failed: %s" %LineIn)
            return False

        return True
    
    def key_parse_9TP(self,LineIn):
        """
        Data from BMP280 in PRS
        Line in follows 
        *9TP,PPPPPP,TTTTTT,^(Check bits may go here some day);
        *9TP,5EC2E0,7E40D0,^;
        """
        
        BMP280ID = "BMP280_2"
        
        if not self.BMP280_line_validation(LineIn):
            return None
        
        
        # Pressure
        PposStart = LineIn.find(",")
        PposEnd = PposStart + LineIn[PposStart+1:].find(",")
            
        # Temperature
        TposStart = PposEnd+1
        TposEnd = TposStart + LineIn[TposStart+1:].find(",")
        
            
        TFine = self.sensor_parse_BME280_TFine(LineIn[TposStart+1:TposEnd+1],BMP280ID)
        
        if(TFine == None):
            data = dict()
            data["pressurePRS"] = None
            return data
        
        Temperature = self.sensor_parse_BME280_Temperature(TFine)
        
        Pressure = self.sensor_parse_BME280_Pressure(LineIn[PposStart+1:PposEnd+1],BMP280ID, TFine)
        
        Temperature = self.trim_Data_Reasonable_Places(Temperature)
        Pressure = self.trim_Data_Reasonable_Places(Pressure)
        
        logdbg("9TP Temperature - Not Recorded: %f" %Temperature)
        logdbg("9TP pressurePRS: %f" %Pressure)
        
        data = dict()
        data["pressurePRS"] = Pressure 
        
        
        return data
        
        
    
    def sensor_parse_TippingBucket_Rain(self,DataHexCurrent,DataHexLast):
        """
        Handles output from a tipping bucket rain gauge. Output is the difference between 
        current reat and last read.
        
        It is assumed that this will read out often enough that a max int of 15 will not be a problem.
        
        Units mm
        
        Usess rain_mm_Per_Tip in weewx.conf for converting bucket tips to mm
        """
        

        try:
            CurrentRainRaw = int(DataHexCurrent,16)
            LastRainRaw = int(DataHexLast,16)
        except (ValueError) as e:
            loginf("RAIN LOST Failed to parse or failed to get a proper hex number, DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
            logerr("RAIN LOST Failed to parse or failed to get a proper hex number, DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
            logmsg(syslog.LOG_EMERG, "RAIN LOST Failed to parse or failed to get a proper hex number, DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
            loginf(traceback.format_exc())
            logerr(traceback.format_exc())
            raise
        
        # Test Line
        logdbg("Sensor_parse_TippingBuckedt_Rain DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
        
        # The counter max int is 15
        if(CurrentRainRaw < LastRainRaw):
            CurrentRainRaw += 16
        
        if(CurrentRainRaw == LastRainRaw):
            return 0.0
        # TODO does this work (float)0
        
                 
        Rain = float(CurrentRainRaw-LastRainRaw) * self.stoic_Cal_dict["rain_mm_Per_Tip"]
        
        #TEST LINE
        loginf("rain %f" % Rain)
        
        return Rain

        
    def key_parse_4R_Rain(self,LineIn):
        """
        Parse key 4R the rainfall since last readout
        Should provide one value in 1 byte HEX with the first 4 bits zeroed eg
        0D
        
        entered as rain
        Units mm
        """
        try:
            posStartCurrent = LineIn.find(",")
            posEndCurrent = posStartCurrent + LineIn[posStartCurrent+1:].find(",")
            posStartLast = posEndCurrent + 1
        
        # Does the line have a checksum indicator
            if(LineIn.find("^") == -1):
                posEndLast = LineIn.find(";")
            else:
                posEndLast = posStartLast + LineIn[posStartLast+1:].find(",") + 1
            
        # TODO add exception if hex lengths are wrong
        # TODO raise exception if first 4 bits are not 0000
        # TODO reduce to half byte printed?
        
            Rain = self.sensor_parse_TippingBucket_Rain(LineIn[posStartCurrent+1:posEndCurrent+1],LineIn[posStartLast+1:posEndLast])
            
            data = dict()
            data["rainSmallTip"] = Rain
            
        except:
            loginf("RAIN LOST Total failure in rain. Rain data may have been lost., LineIn: %s" %LineIn)
            logerr("RAIN LOST Total failure in rain. Rain data may have been lost., LineIn: %s" %LineIn)
            logmsg(syslog.LOG_EMERG, "RAIN LOST Total failure in rain. Rain data may have been lost., LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            logerr(traceback.format_exc())
            raise
        
        # TODO test above. The last time it happened it spit an error. I changed LOG_EMERG to syslog.LOG_EMERG. THis has not been tested.
        # Test by having the arduino spit an invalid line with 4R in it.
        
        return data
    
    def sensor_parse_wind_direction_from_ADC(self,WindADCRaw):
        """
        Takes an int
        """
        
        # The wind model uses a main, high, and low part
        if WindADCRaw < self.stoic_Cal_dict["wind_direction_Low_Model_Cutoff"]:
            Model = "Low"
        elif WindADCRaw > self.stoic_Cal_dict["wind_direction_High_Model_Cutoff"]:
            Model = "High"
        else:
            Model = "Main"
            
        Offset = "wind_direction_cal_" + Model + "_Model_offset"
        slope = "wind_direction_cal_" + Model + "_Model_slope"
            
        WindDirectionDeg = (float(WindADCRaw) - self.stoic_Cal_dict[Offset]) / self.stoic_Cal_dict[slope]
        
        
        logdbg("sensor_parse_wind_direction_from_ADC WindADCRaw Model WindDirectionDeg %d %s, %f" % (WindADCRaw, Model, WindDirectionDeg))
        
        return WindDirectionDeg
    
        
    
    def sensor_parse_wind_direction_gust(self,LineIn_5WGD):
        """
        Wind direction is reported as a raw ADC value which has been rotated through the compass directions.
        
        
        """
        DirectionRaw = int(LineIn_5WGD,16)
        
        
        WindDirection = self.sensor_parse_wind_direction_from_ADC(DirectionRaw)
        
       
        
        return WindDirection
        
        
    def sensor_parse_wind_speed_gust(self,LineIn_6WGS):
        """
        The gust speed is measured at 2.25 second intervals. One clikc per second is 1 mph. Thus
        
        """
        
        GustSpeedRaw = int(LineIn_6WGS,16)
        
        GustSpeed = float(GustSpeedRaw) * self.stoic_Cal_dict["wind_mps_per_click_per_2_25_s"]
        
        
        return GustSpeed
        
        
    def wind_gust_line_validation(self,LineIn):
        if LineIn.find("*6WGS") == -1:
            logdbg("wind_gust_line_validation no *6WGS")
            return False
        pos = LineIn.find(";")
        if pos == -1:
            logdbg("wind_gust_line_validation no ;")
            return False
        if LineIn[pos:].find("+5WGD") == -1:
            logdbg("wind_gust_line_validation no +5WGD")
            return False
        if LineIn[pos+1:].find(";") == -1:
            logdbg("wind_gust_line_validation no second ;")
            return False
        
       # TODO add no speeds > 1023 ADC
        
        return True
    
    
    def key_parse_6WGS_5WGD_wind_gust(self,LineIn):
        """
        The wind gust formatt is *6WGS,1A;+5WGD,141; Speed is clicks per 2.25 seconds direction is 
        its usual mess.
        """

        if not self.wind_gust_line_validation(LineIn):
            loginf("key_parse_5WGD_5WGS_WindGust received an invalid line %s" % LineIn)
            return None
        
        posStart = LineIn.find(",") + 1
        posEnd = LineIn.find(";")
        
        
        GustSpeed = self.sensor_parse_wind_speed_gust(LineIn[posStart:posEnd])
        
        
        posStart = LineIn[posEnd+1:].find(",") + posEnd +1 +1
        posEnd = LineIn[posEnd+1:].find(";") + posEnd +1
        
        
        GustDirection = self.sensor_parse_wind_direction_gust(LineIn[posStart:posEnd])
        
        if GustDirection > 360:
            loginf("GustDirection is a tad off %f" % GustDirection)
            
        if GustDirection < 0:
            loginf("GustDirection is a tad off %f" % GustDirection)
        
        
        GustDirection = self.trim_Data_Reasonable_Places(GustDirection)
        GustSpeed = self.trim_Data_Reasonable_Places(GustSpeed)
        
        data = dict()
        data["windGustDir"] = GustDirection
        data["windGust"] = GustSpeed
        
        # TEST lines
        loginf(LineIn)
        loginf("key_parse_6WGS_5WGD_wind_gust windGustDir %f" % data["windGustDir"])
        loginf("key_parse_6WGS_5WGD_wind_gust windGust %f" % data["windGust"])
        
        return data
    
    

    def sensor_parse_wind_direction_mean(self,LineInDirection,LineInBin):
        """
        Mean direction is tricky. The Stoic Watcher bins and data and adds a rotation. Then it sums the the rotated readings.
        
        Here we devide by the total number of readings and un rotate.
        """
        

        
        DirectionRaw = int(LineInDirection,16)
        BinNumber = int(LineInBin,16)
        

        # Divide to take mean
        DirectionMeanRaw = DirectionRaw / self.stoic_Cal_dict["wind_direction_number_of_mean_points"]
        
        # unrotate
        if(BinNumber >= (self.stoic_Cal_dict["wind_direction_number_of_bins"] / 2)):
            rotation = 1535 - ((BinNumber * 64) + self.stoic_Cal_dict["wind_direction_half_bin_size"])
        else:
            rotation = 511 - ((BinNumber * 64) + self.stoic_Cal_dict["wind_direction_half_bin_size"])
        
        

        
        DirectionMeanRaw = DirectionMeanRaw - rotation

        
        if DirectionMeanRaw < 0:
            DirectionMeanRaw += 1024
        
        if DirectionMeanRaw > 1023:
            DirectionMeanRaw -= 1024

       
        WindDirectionMean = self.sensor_parse_wind_direction_from_ADC(DirectionMeanRaw)
        
        if WindDirectionMean > 360:
            loginf("sensor_parse_wind_direction_mean this should never happen > 360")
            WindDirectionMean -= 360
            
        if WindDirectionMean < 0:
            loginf("sensor_parse_wind_direction_mean this should never happen < 0")
            WindDirectionMean += 360
        
        
        return WindDirectionMean
    
        
    def sensor_parse_wind_speed_mean(self,LineIn_6WMS):
        """
        The wind mean is a simple thing. The Stoic Watcher sends the difference between the current count and the count at time interval ago.
        V = P(2.25/T)
        For the davis anemometer, speed (mph) = counts * 2.25  / time of sample
        We report speed in m/s here
        
        """

        
        MeanSpeedRaw = int(LineIn_6WMS,16)

        
        MeanSpeed = float(MeanSpeedRaw) * self.stoic_Cal_dict["wind_mps_per_click_per_120_s"]
        
        
        return MeanSpeed
        
    def wind_mean_line_validation(self,LineIn):
        if LineIn.find("*6WMS") == -1:
            logdbg("wind_mean_line_validation no *6WMS")
            return False
        pos = LineIn.find(";")
        if pos == -1:
            logdbg("wind_mean_line_validation no ;")
            return False
        if LineIn[pos:].find("+5WMD") == -1:
            logdbg("wind_mean_line_validation no +5WMD")
            return False
        if LineIn[pos+1:].find(";") == -1:
            logdbg("wind_mean_line_validation no second ;")
            return False
        
        # +5WMD,0F531,08;  seems to be the driection read out when nothing is plugged in. Consider as part of line validation
        
        
        return True
        
    def key_parse_6WMS_5WMD_wind_mean(self,LineIn):
        """
        Wind comes on one line *6WMS,<data>;+5WMD,<data>;
    
        No wind is recorded as 0 deg direction. 360 is for north with wind.
        """ 
        if not self.wind_mean_line_validation(LineIn):
            loginf("key_parse_6WMS_5WMD_wind_mean received an invalid line %s" % LineIn)
            return None
        
        posStart = LineIn.find(",") + 1
        posEnd = LineIn.find(";")
        

        
        MeanSpeed = self.sensor_parse_wind_speed_mean(LineIn[posStart:posEnd])
        
        posSecondBlock = LineIn.find("+")
        
        posStart = LineIn[posSecondBlock+1:].find(",") + posSecondBlock +1 +1
        posMiddle = LineIn[posStart:].find(",") + posStart 
        posEnd = LineIn[posSecondBlock+1:].find(";") + posSecondBlock +1

        
        
        MeanDirection = self.sensor_parse_wind_direction_mean(LineIn[posStart:posMiddle],LineIn[posMiddle +1:posEnd])
        
        if MeanDirection > 360:
            loginf("MeanDirection is a tad off %f" % MeanDirection)
            
        if MeanDirection < 0:
            loginf("MeanDirection is a tad off %f" % MeanDirection)
            
        # These are rules from WeeWX
        if (MeanDirection == 0) and (MeanSpeed > 0):
            MeanDirection = 360
        if MeanSpeed == 0:
            MeanDirection = 0
            
        MeanDirection = self.trim_Data_Reasonable_Places(MeanDirection)
        MeanSpeed = self.trim_Data_Reasonable_Places(MeanSpeed)
        
        
        
        data = dict()
        data["windDir"] = MeanDirection
        data["windSpeed"] = MeanSpeed
        
        # TEST lines
        loginf(LineIn)
        loginf("key_parse_6WMS_5WMD_wind_mean windDir %f" % data["windDir"])
        loginf("key_parse_6WMS_5WMD_wind_mean windSpeed %f" % data["windSpeed"])
        
        # Gusts are defined such that there may not always be a gust. StoicWatcher does not send a WGS line when there is no gust.
        #  This will cause Weewx to assum that gusts are not reported at all if it does not get one in an archive intervaul period. 
        #  We don't care about a mean gust. The archive record will take the max of windgust to archive. 
        #  So here we fake it so WeeWx knows it is getting gust data from the station.
        data["windGust"] = float(0.0)
        data["windGustDir"] = float(0.0)
        
        return data
    
    def sensor_parse_DS18B20_1Wire(self,DataHex):
        """
        Handles 1 Wire data from DS18B20. 
        Data format 014D
        Bit 15 - 11 inclusive are sign bits
        Bit 10 2^6
        Bit  7 2^3
        Bit  02^-4
        twos compliment
        """
        
        DataRaw = int(DataHex,16)
        
        # Check sign
        if ( (DataRaw >> 12) & int("1",16) ) == 1:
            # Negative
            Temp = (float(DataRaw & int("07FF",16)) * (2**-4)) - 128.0
        else:
            #positive
            Temp = float(DataRaw & int("07FF",16)) * (2**-4)
            
        return Temp
            
        
    
    def temp_1Wire_line_validation(self, LineIn):
        
        pos = LineIn.find(",")
        
        if LineIn[0:2] != "*2":
            logdbg("temp_1Wire_line_validation no *2")
            return False
        if LineIn[3:pos] != "T":
            logdbg("temp_1Wire_line_validation no T,")
            return False
        
        if LineIn.find("^") == -1:
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find("^")
        
        if (posEnd - pos) != 5:
            logdbg("temp_1Wire_line_validation data not correct length")
            return False
        
        #  85 C is the reset value for DS18B20. This value should be ignored. 0x0550
        if LineIn.find("0550") != -1:
            loginf("temp_1Wire_line_validation read is 85C, the reset value of the DS18B20 %s" %LineIn)
            return False
        
        return True
        

    
    def key_parse_2xT_1WireTemp(self, LineIn):
        """
        Handle the 1 Wire temp sensors 2xT where x = {0...7}
        *21T,014D;
        """
        
        if not self.temp_1Wire_line_validation(LineIn):
            return None
        
        pos = LineIn.find(",")
        
        if LineIn.find("^") == -1:
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find("^")
        
        Temp = self.sensor_parse_DS18B20_1Wire(LineIn[pos+1:posEnd])

        if not self.result_check_temp(Temp):
            return None
        
        Temp = self.trim_Data_Reasonable_Places(Temp)
        
        
        data = dict()
        data[self.stoic_Cal_dict[ "Temp1W-schema-" + LineIn[2:3]] ] = Temp
             
        loginf("key_parse_2xT_1WireTemp LineIn: %s key %s value %f " %(LineIn, self.stoic_Cal_dict[ "Temp1W-schema-" + LineIn[2:3]], data[self.stoic_Cal_dict[ "Temp1W-schema-" + LineIn[2:3]] ]))
        
        if self.stoic_Cal_dict[ "Temp1W-schema-" + LineIn[2:3]] == "NothingYet":
            return None
        
        return data
    
    def sensor_parse_AS3935_Lightning(self, DataHex):
        """
        AS3935 Provides distance in km.
        Units km
        """
        # This is left here as a monument to not being able to do binary
        # Hex look up table for distance
        #DistanceLUT = dict()
        #data["28"] = int(40)
        #data["25"] = int(37)
        #data["22"] = int(34)
        #data["1F"] = int(31)
        #data["1B"] = int(27)
        #data["18"] = int(24)
        #data["14"] = int(20)
        #data["11"] = int(17)
        #data["0E"] = int(14)
        #data["0C"] = int(12)
        #data["0A"] = int(10)
        #data["08"] = int(8)
        #data[""] = int()
        #data[""] = int()
        
        Distance = int(DataHex,16)
        
        if DataHex == "3F":
            loginf("sensor_parse_AS3935_Lightning Out of range")
            return None
        if DataHex == "01":
            loginf("sensor_parse_AS3935_Lightning Directly Overhead")
            Distance = int(0)
            
        return Distance
        
        
        
    
    def key_parse_11EM_Lightning(self, LineIn):
        """
        Handles the data from an AS3935 lightning detector.
        *11EM,3F;
        
        """
        pos = LineIn.find(",")
        posEnd = LineIn.find(";")
        
        Distance = sensor_parse_AS3935_Lightning(LineIn[pos+1:posEnd])
        
        loginf("sensor_parse_AS3935_Lightning Distance %d" %Distance)
        
        data = dict()
        data["LightningDistance"] = Distance
        
        return data
    
    def sensor_parse_ChipCap2_Temp(self, DataHex):
        """
        Parse the two hex bytes output from a ChipCap2 tempriture sensor.
        Returns tempriture C
        
        (Temp_High [7:0] x 64 + Temp_Low [7:2]/ 4)/ 214 x 165 - 40
        """
        
        logdbg("sensor_parse_ChipCap2_Temp DataHex: %s" %DataHex)
        
        DataRaw = int(DataHex,16)
        
        logdbg("sensor_parse_ChipCap2_Temp DataRaw: %d" %DataRaw)
        
        # Bottom Two bits are garbage
        DataRaw = DataRaw & int("FFFC",16)
        
        Temp = ((DataRaw >> 8) * 64) + ((DataRaw & int("FC",16)) >> 2)
        Temp = float(Temp) / 16384.0
        Temp = Temp * 165.0
        Temp = Temp - 40.0

        return Temp
    
    def sensor_parse_ChipCap2_Humid(self, DataHex):
        """
        Parse the two hex bytes output from an ChipCap2 TH sensor.
        Returns RH
        
        (RH_High [5:0] x 256 + RH_Low [7:0])/ 214 x 100
        """
        
        logdbg("sensor_parse_ChipCap2_Humid DataHex: %s" %DataHex)
        
        DataRaw = int(DataHex,16)
        
        logdbg("sensor_parse_ChipCap2_Humid DataRaw: %d" %DataRaw)
        
        # Top two bits are garbage
        DataRaw = DataRaw & int("3FFF",16)
        
        RelativeHumidity = ((DataRaw >> 8) * 256) + (DataRaw & int("FF",16))
        RelativeHumidity = float(RelativeHumidity) / 16384.0
        RelativeHumidity = RelativeHumidity * 100.0
            
        return RelativeHumidity
    
    
    
    def temp_Hu_8TH_line_validation(self, LineIn):
        """

TH,00000000
No I2C connection
        """
        
        pos = LineIn.find(",")
        
        if LineIn.find("^") == -1:
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find("^")
        
        if (posEnd - pos) != 9:
            logdbg("temp_Hu_8TH_line_validation data not correct length")
            logdbg("temp_Hu_8TH_line_validation LineIn: %s" %LineIn)
            return False
        
        if(LineIn.find("TH,00000000") != -1):
            logdbg("temp_Hu_8TH_line_validation data all zeros - likely no sensor conneciton")
            logdbg("temp_Hu_8TH_line_validation LineIn: %s" %LineIn)
            return False
        
        return True
    
    def key_parse_8TH(self, LineIn):
        """
        *8TH,16C05D98; 
        Handles the tempreture and humidity sensor in the PRS
        ChipCap2 type sensor
        HHHHTTTT
        """
        
        
        
        if not self.temp_Hu_8TH_line_validation(LineIn):
            return None
        
        pos = LineIn.find(",")
        
        if LineIn.find("^") == -1:
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find("^")
            
        Humidity = self.sensor_parse_ChipCap2_Humid(LineIn[pos+1:pos+1+4])
            
        Temperature = self.sensor_parse_ChipCap2_Temp(LineIn[pos+1+4:posEnd])
        
        Temperature = self.trim_Data_Reasonable_Places(Temperature)
        Humidity = self.trim_Data_Reasonable_Places(Humidity)
        
        logdbg("8TH TempPRS: %f" %Temperature)
        logdbg("8TH HumidityPRS: %f" %Humidity)
        
        data = dict()
        data["TempPRS"] = Temperature
        data["HumidityPRS"] = Humidity
        return data
    
    def sensor_parse_MLX90614_IR_Temp(self,DataHex):
        """
        MLX90614 IR Sensor
        
        T (kelven) = DataHex * 0.02 K
        """
        
        DataRaw = int(DataHex,16)
        
        Temperature = float(DataRaw)
        
        Temperature = (Temperature * 0.02) - 273.15
        
        return Temperature
        
    
    def IR_soil_temp_line_validation(self,LineIn):
        
        pos = LineIn.find(",")
        
        if LineIn.find("^") == -1:
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find("^")
        
        if (posEnd - pos) != 5:
            loginf("IR_soil_temp_line_validation failed: %s" %LineIn)
            return False
        
        return True
    
    
    def key_parse_30T(self, LineIn):
        """
        *30T,xxxx; 
        IR Soil Temp
        """
        
        logdbg("30T LineIn: %s" %LineIn)
        
        
        if not self.IR_soil_temp_line_validation(LineIn):
            return None
        
        pos = LineIn.find(",")
        
        if LineIn.find("^") == -1:
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find("^")
            
        Temperature = self.sensor_parse_MLX90614_IR_Temp(LineIn[pos+1:posEnd])
        
        
        Temperature = self.trim_Data_Reasonable_Places(Temperature)
        
        logdbg("30T IR Soil: %f" %Temperature)
        
        data = dict()
        data["soilTempIR"] = Temperature
        return data
            
        
        
    
    def parse_raw_data(self, LineIn):
        if LineIn[0] != "*":
            logdbg("parse_raw_data given bad data")
            return None
        
        # Find the key
        pos = LineIn.find(",")
        
        # Call the correct parser based on the key
        try:
        #Temperature in the circuit box
            if LineIn[1:pos] == "2T":
                return self.key_parse_2T_BoxTemp(LineIn)
            elif LineIn[1:pos] == "3TPH":
                return self.key_parse_3TPH_FARS(LineIn)
            elif LineIn[1:pos] == "4R":
                return self.key_parse_4R_Rain(LineIn)
            elif LineIn[1:pos] == "6WGS":
                return self.key_parse_6WGS_5WGD_wind_gust(LineIn)
            elif LineIn[1:pos] == "6WMS":
                return self.key_parse_6WMS_5WMD_wind_mean(LineIn)
            elif LineIn[1:pos] == "7T":
               return self.key_parse_7T_FARSTemp(LineIn)
            elif ( LineIn[1:2] == "2" ) and ( LineIn[3:pos] == "T" ):
                return self.key_parse_2xT_1WireTemp(LineIn)
            elif LineIn[1:pos] == "11EM":
               return self.key_parse_11EM_Lightning(LineIn)
            else:
                try:
                    return getattr(self, 'key_parse_' + LineIn[1:pos])(LineIn)
                except (AttributeError) as e:
                    loginf("AttributeError - parse_raw_data - Cannot find function to handle, LineIn: %s" %LineIn)
                    logerr("AttributeError - parse_raw_data - Cannot find function to handle, LineIn: %s" %LineIn)
                    loginf(traceback.format_exc())
                    return None
        except (ValueError) as e:
            loginf("ValueError - parse_raw_data - Failed to get a proper hex number?, LineIn: %s" %LineIn)
            logerr("ValueError - parse_raw_data - Failed to get a proper hex number?, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (LookupError) as e:
            loginf("LookupError - parse_raw_data - LineIn: %s" %LineIn)
            logerr("LookupError - parse_raw_data - LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (IndexError, KeyError) as e:
            loginf("IndexError, KeyError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            logerr("IndexError, KeyError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (ArithmeticError) as e:
            loginf("ArithmeticError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            logerr("ArithmeticError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (TypeError) as e:
            loginf("TypeError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            logerr("TypeError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        
            
    
    @staticmethod
    def validate_string(LineIn):
        # TODO add aditional validation
        
        
        if (len(LineIn)<1):
            raise weewx.WeeWxIOError("Unexpected line length %d" % len(LineIn))
    
        # Rain cannot be easily recovered if lost. 
        if(LineIn.find("4R") != -1):
            if(LineIn.find("4R") != 1):
                loginf("RAIN LOST validate_string found '4R' out of place. Rain data may have been lost., LineIn: %s" %LineIn)
                logerr("RAIN LOST validate_string found '4R' out of place. Rain data may have been lost., LineIn: %s" %LineIn)
                logmsg(syslog.LOG_EMERG, "RAIN LOST validate_string found '4R' out of place. Rain data may have been lost., LineIn: %s" %LineIn)
                # TODO attempt recovery by reading the rain
                raise weewx.WeeWxIOError(" '4R' is out of place. Rain data may have been lost %s" % LineIn)
            # TODO test this. The last time it happened it spit an error. I changed LOG_EMERG to syslog.LOG_EMERG. THis has not been tested.
        # Test by having the arduino spit an invalid line with 4R in it.
        
        if(LineIn.find(";") == -1):
            raise weewx.WeeWxIOError("Line lacks terminator ; %s" % LineIn)
        if(LineIn[1:].find("*") != -1):
            raise weewx.WeeWxIOError("Line has extra start * %s" % LineIn)
        
        pos = LineIn[1:].find("+")
        if pos != -1:
            if(LineIn[pos:].find(";") == -1):
                raise weewx.WeeWxIOError("Line lacks second terminator ; %s" % LineIn)
            
        if LineIn[0:1] == "+":
            raise weewx.WeeWxIOError("Line starts with continuation %s" % LineIn)
        
        # Check line starts
        ValidLineStarts = set(["*","#","!"])
        if not (LineIn[0:1] in ValidLineStarts):
            raise weewx.WeeWxIOError("Line lacks valid line start *,#,! %s" % LineIn)
        
        #Only for data strings that start with data cue
        if(LineIn.find("*") == 0):
            if(LineIn.find(",") == -1):
                raise weewx.WeeWxIOError("Line lacks seporator , %s" % LineIn)
    

    def check_for_disabled_sensors(self, LineIn):
        """
        Checks the current line against _ListOfDisabledSensors
        False if disabled
        """
        pos = LineIn.find(",")
        
        #TODO Disable sensors when the Arduino says they are bad

        if LineIn[1:pos] in self._ListOfDisabledSensors:
            return False
        else:
            return True
        
    
    
    def get_raw_data_with_retry(self, max_tries=10, maxTrysBeforeCloseAndOpenPort=8, retry_wait=3):
        for ntries in range(0, max_tries):
            try:
                LineIn = self.get_raw_data()
                self.validate_string(LineIn)
                return LineIn
            except (serial.serialutil.SerialException, weewx.WeeWxIOError) as e:
                loginf("Failed attempt %d of %d to get readings: %s" %
                       (ntries + 1, max_tries, e))
                time.sleep(retry_wait)
                if ntries > maxTrysBeforeCloseAndOpenPort:
                    loginf("No Good. Killing the port.")
                    self.close()
                    self.open()
        else:
            msg = "Max retries (%d) exceeded for readings" % max_tries
            logerr(msg)
            self.send_reset_signal_to_Arduino()
            #TODO impliment the above better
            raise weewx.RetriesExceeded(msg)
        
    def get_processed_data_with_retry(self, max_tries=25, maxTrysBeforeCloseAndOpenPort=8, retry_wait=3):
        DataLine = False
        while not DataLine:
            LineIn = self.get_raw_data_with_retry(max_tries, maxTrysBeforeCloseAndOpenPort, retry_wait)
            
            if LineIn[0] == "*":
                # Data Line
                DataLine = True
                
            elif LineIn[0] == "!":
                # A computer readable alert
                # TODO Deal with computer readable alerts
                loginf('LineIn: %s' % LineIn)
                self.alert_parse(LineIn)
            
            elif LineIn[0] == "#":
                loginf('%s' % LineIn)
            #else:
                # Not a valid line, validate_string should eleminate these but simply ignored here
                
            if not self.check_for_disabled_sensors(LineIn):
                loginf('Sensor Diaabled: %s' % LineIn)
                DataLine = False
        
        ParsedData = self.parse_raw_data(LineIn)
        
        if ParsedData == None:
            loginf('Unable to parse %s' % LineIn)
        
        return ParsedData
    
    def send_reset_signal_to_Arduino(self):
        # send serial reset to Arduino
        # TODO impliment send_reset_signal_to_Arduino
        loginf("Attempting serial reset of Arduino")
        
        # Send Reset signal
        
        # Varify data received
        # raise exception if unable to reset
            
    def alert_parse(self,LineIn):
        """
        Handle lines starting with !
	self.local_log_file = stn_dict.get('local_log_file', None)
	self.save_bang_Ard = int(stn_dict.get('save_bang', 0))
        """
	if self.save_bang_Ard:
            timeHolder = int(time.time())
            loginf('Time of Special Log %d' % timeHolder)
            with open(self.local_log_file, "a") as SpecialLogFile: 
                SpecialLogFile.write(time.strftime("%Y-%m-%d-%H:%M:%S,", time.gmtime()))
                #This is they key used for data records
                SpecialLogFile.write("%d," %timeHolder)
                SpecialLogFile.write(LineIn)
                SpecialLogFile.write("\n")
	    
            
        




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
        print "Typically /dev/ttyACM1 for Arduino connected to R Pi"
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
                      default=StoicWatcher.DEFAULT_PORT)
    (options, args) = parser.parse_args()

    if options.version:
        print "StoicWS driver version %s" % DRIVER_VERSION
        exit(0)

    with StoicWatcher(options.port, debug_serial=options.debug) as StoicWatcher:
        while True:
            print time.time(), _fmt(StoicWatcher.get_raw_data())
    
