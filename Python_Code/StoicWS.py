#!/usr/bin/env python
#
# Stoic WS
# Version 0.0.11
# 2018-02-08
#
# This is a driver for weeWX to connect with an Arduino based weather station.
# see
#http://www.weewx.com/docs/customizing.htm
#Under Implement the driver

"""weeWX Driver for Stoic Weather Watch.
An Arduino based weather station.

Stoic sends data constantly via a USB serial port. 

Data format
*Key,Value(s);
Items on the same line
*Key,Value(s);+key,Value(s);

Supported Keys:                                    Mapped DB name
2T Temperature in Box  -                           extraTemp1
3TPH Temperature, pressure, humidity in FARS -     outTemp, pressure, outHumidity
   This is BME280ID "BME280-1"
4R Rain tipping bucket - rain
   Reported since last report - a difference
5WMD Wind direction mean
6WMS Wind speed mean
      6WMS and 5WMD are expected on the same line. (int
5WGD Wind gust direction. Reported as 4 readings which can be averaged.
6WGS wind speed gust. Reported as current, min, max. From 54 records. At 2.25 seconds 121.5 seconds
      6WGS and 5WGD are expected on the same line. 


Alert format
!Key,Value;

All other line starts ignored, including #

Units used are weewx.METRICWX

CHIP from NEXTTHING sticks an arduino on a USB hub at ttyACM1
DEFAULT_PORT = "/dev/ttyACM1"

DEFAULT_BAUDRATE = 9600

Looks for the following in the config file
rain_mm_Per_Tip - Tipping bucket conversion factor 0.2794

"""


# TODO First readout after !Startup; is not reported. Used as baseline for wind etc.

# TODO If your lose the  connection, you should raise an exception of type 
# weewx.WeeWxIOError, or a subclass of weewx.WeeWxIOError. The engine will catch this exception 
# and, by default, after 60 seconds do a restart. This will cause your driver to reload, giving it an 
# opportunity to reconnect with the broker.

#
#Device drivers should be written to emit None if a data value is bad 
#(perhaps because of a failed checksum). If the hardware simply doesn't 
#support it, then the driver should not emit a value at all.

# TODO crashes when Aruino dissconnected. Handle gracefully

# TODO cat /sys/class/thermal/thermal_zone0/temp  / 1000 gives temp C of CPU CHIP
# TODO add chip temp for Arduino

# TODO truncate T, P, and H at two decimal places. Anything more is past precision limits
# TODO eleminate *3TPH,000000,000000,0000,^; and report an error instead


# TODO DO I need this? Python 2.7 should have with_statement
from __future__ import with_statement

import serial
import syslog
import time
import traceback
import serial.tools.list_ports

# TODO do I use binascii?
import binascii

import weewx.drivers

DRIVER_NAME = 'StoicWS'
DRIVER_VERSION = '0.0.11'

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
            
            loginf("BoschHEXHEX2UnsignedLong msb %s " % msb)
            loginf("BoschHEXHEX2UnsignedLong lsb %s " % lsb)
           
            return  ((long(msb,16) << 8) + long(lsb,16))
            
        def BoschHEXHEX2SignedLong(msb,lsb):
            
            if((long(msb,16) >> 7) == 1):
                sign = long(-1)
            else:
                sign = long(1)
                
            
            return (sign * (((long(msb,16) & 0b01111111) << 8) + long(lsb,16)))
        
        stoic_Cal_dict = dict()
         
         # TEST line
        loginf("StoicWSDriver.readCalibrationDict running")
         
        # TODO make this a try so it fails gracefully when not in the dictionary
        
        #TODO Handle a value not present gracefully
        #try:
        
        # General
        # Serial
        stoic_Cal_dict["serial_port_default"] = stn_dict.get('serial_port_default')
        stoic_Cal_dict["serial_port_Prefix"] = stn_dict.get('serial_port_Prefix')
        stoic_Cal_dict["serial_port_Lowest"] = int(stn_dict.get('serial_port_Lowest'))
        
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
        
        loginf("self.stoic_Cal_dict['wind_direction_cal_Main_Model_offset'] %f" % stoic_Cal_dict["wind_direction_cal_Main_Model_offset"])
        loginf("stoic_Cal_dict['wind_direction_cal_Main_Model_slope'] %f" % stoic_Cal_dict["wind_direction_cal_Main_Model_slope"])
        
        loginf("stoic_Cal_dict['wind_mps_per_click_per_2_25_s'] %f" % stoic_Cal_dict["wind_mps_per_click_per_2_25_s"])
        loginf("stoic_Cal_dict['wind_direction_dead_zone_dir'] %f" % stoic_Cal_dict["wind_direction_dead_zone_dir"])
        
        # BME280 A 3TPH "BME280-1"
        # The BME280 has lots of calibration data which is hard coded in the factory. 
        # The calculations require at least 64 bit integer math
        # Not easily automated at read in either...
        #  Data sheet T1 Unsigned 
        #stoic_Cal_dict["BME280_1_CAL_T1"] = (long(stn_dict.get("cal-BME280-1.2.1"),16) << 8) + long(stn_dict.get("cal-BME280-1.2.0"),16)
        
        # TEST Lines
        #loginf("stn_dict.get('cal-BME280-1.2.1') %s" % stn_dict.get("cal-BME280-1.2.1"))
        #loginf("stn_dict.get('cal-BME280-1.2.0') %s" % stn_dict.get("cal-BME280-1.2.0"))
        
        stoic_Cal_dict["BME280_1_CAL_T1"] = BoschHEXHEX2UnsignedLong(stn_dict.get("cal-BME280-1.2.1"),stn_dict.get("cal-BME280-1.2.0"))
        # TEST LINE
        #loginf("stoic_Cal_dict['BME280_1_CAL_T1'] %d" % stoic_Cal_dict["BME280_1_CAL_T1"])
        #  Data sheet T2 signed 
        #stoic_Cal_dict["BME280_1_CAL_T2"] = ((long(stn_dict.get("cal-BME280-1.2.3"),16) >> 7) * long(-1) ) * (((long(stn_dict.get("cal-BME280-1.2.3"),16) & 0b01111111) << 8) + long(stn_dict.get("cal-BME280-1.2.2"),16))
        stoic_Cal_dict["BME280_1_CAL_T2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.3"),stn_dict.get("cal-BME280-1.2.2"))
         # TEST LINE
        #loginf("stoic_Cal_dict['BME280_1_CAL_T2'] %d" % stoic_Cal_dict["BME280_1_CAL_T2"])
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
        #stoic_Cal_dict["BME280_1_CAL_P8"] = ((long(stn_dict.get("cal-BME280-1.2.21"),16) >> 7) * long(-1) ) * (((long(stn_dict.get("cal-BME280-1.2.21"),16) & 0b01111111) << 8) + long(stn_dict.get("cal-BME280-1.2.20"),16))
        stoic_Cal_dict["BME280_1_CAL_P8"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.21"),stn_dict.get("cal-BME280-1.2.20"))
        # TEST LINE
        loginf("stoic_Cal_dict['BME280_1_CAL_P8'] %d" % stoic_Cal_dict["BME280_1_CAL_P8"])
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
        # TEST LINE
        loginf('stn_dict.get("cal-BME280-1.1.5") %s' % stn_dict.get("cal-BME280-1.1.5") )
        loginf('stn_dict.get("cal-BME280-1.1.4") %s' % stn_dict.get("cal-BME280-1.1.4") )
        loginf('long(stn_dict.get("cal-BME280-1.2.5"),16) %d' % long(stn_dict.get("cal-BME280-1.1.5"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.4"),16) %d' % long(stn_dict.get("cal-BME280-1.1.4"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.5"),16) %X' % long(stn_dict.get("cal-BME280-1.1.5"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.4"),16) %X' % long(stn_dict.get("cal-BME280-1.1.4"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.1.5"),16) << 4 %d' % (long(stn_dict.get("cal-BME280-1.1.5"),16) << 4))
        loginf('((long(stn_dict.get("cal-BME280-1.1.5"),16) & 0b01111111) << 4) %X' % ((long(stn_dict.get("cal-BME280-1.1.5"),16) & 0b01111111) << 4))
        loginf('(long(stn_dict.get("cal-BME280-1.1.4"),16) >> 4) %X' % (long(stn_dict.get("cal-BME280-1.1.4"),16) >> 4))
        loginf("stoic_Cal_dict['BME280_1_CAL_H5'] %d" % stoic_Cal_dict["BME280_1_CAL_H5"])
        
        
        #  Data sheet H6 signed byte E7
        sign = -1 if (long(stn_dict.get("cal-BME280-1.1.6"),16) >> 7 == 1) else 1
        stoic_Cal_dict["BME280_1_CAL_H6"] = long(sign) * (long(stn_dict.get("cal-BME280-1.1.6"),16) & 0b01111111)
        # TEST LINE
        loginf("stoic_Cal_dict['BME280_1_CAL_H6'] %d" % stoic_Cal_dict["BME280_1_CAL_H6"])
        loginf("stoic_Cal_dict['BME280_1_CAL_H6'] %X" % stoic_Cal_dict["BME280_1_CAL_H6"])

        
        # Test stuff
        for i in range(1,3+1):
             loginf("stoic_Cal_dict['BME280_1_CAL_T" + str(i) +"  0x%X" % stoic_Cal_dict["BME280_1_CAL_T"+str(i)])
        for i in range(1,9+1):
             loginf("stoic_Cal_dict['BME280_1_CAL_P" + str(i) +"  0x%X" % stoic_Cal_dict["BME280_1_CAL_P"+str(i)])
        for i in range(1,6+1):
             loginf("stoic_Cal_dict['BME280_1_CAL_H" + str(i) +"  0x%X" % stoic_Cal_dict["BME280_1_CAL_H"+str(i)])

        
        return stoic_Cal_dict
    

    def __init__(self, **stn_dict):
        self.model = stn_dict.get('model', 'StoicWS')
        # TODO Figure out the port
        self.port = stn_dict.get('serial_port_default', StoicWatcher.DEFAULT_PORT)
        self.baudrate = stn_dict.get('baudrate', StoicWatcher.DEFAULT_BAUDRATE)
        self.max_tries = int(stn_dict.get('max_tries', 25))
        self.retry_wait = int(stn_dict.get('retry_wait', 3))
        debug_serial = int(stn_dict.get('debug_serial', 0))
        
        #No need for this. The hardware resets the rain count when the serial port is reset.
        # Also the hardware reports its own last rain
        self.last_rain = None

        loginf('driver version is %s' % DRIVER_VERSION)
        loginf('using serial port %s' % self.port)
        
        #TEST LINE
        loginf("stn_dict.get('rain_mm_Per_Tip') %s" % stn_dict.get('rain_mm_Per_Tip'))
        # TEST Lines
        loginf("stn_dict.get('cal-BME280-1.2.1') %s" % stn_dict.get("cal-BME280-1.2.1"))
        loginf("stn_dict.get('cal-BME280-1.2.0') %s" % stn_dict.get("cal-BME280-1.2.0"))
        
        stoic_Cal_dict = self.readCalibrationDict(stn_dict)
        #TEST LINE
        loginf("stoic_Cal_dict[rain_mm_Per_Tip] %f" % stoic_Cal_dict["rain_mm_Per_Tip"])
        

        self.StoicWatcher = StoicWatcher(self.port, self.baudrate, stoic_Cal_dict, debug_serial=debug_serial)
        #self.StoicWatcher = StoicWatcher(self.port, self.baudrate, debug_serial=debug_serial)
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
            
            if data != None:
                packet.update(data)
                yield packet

class StoicWatcher(object):
    
    DEFAULT_PORT = "/dev/ttyACM1"
    DEFAULT_BAUDRATE = 9600
    
    def __init__(self, port, baudrate, stoic_Cal_dict, debug_serial=0):
    #def __init__(self, port, baudrate, debug_serial=0):
        self._debug_serial = debug_serial
        self.port = port
        self.baudrate = baudrate
        self.timeout = 3 # seconds
        self.serial_port = None
        
        self.stoic_Cal_dict = stoic_Cal_dict

        
    def __enter__(self):
        self.open()
        return self

    def __exit__(self, _, value, traceback):
        self.close()
        
    def open(self):
        
        # TODO validate that we are talking to the Arduino and not something else
        
        if len(list(serial.tools.list_ports.comports())) == 0:
            loginf("Stoic: No serial ports found")
        
        NumTrys = 15
        PortNum = self.stoic_Cal_dict["serial_port_Lowest"]
        for i in range(NumTrys):
            try:
                loginf("Stoic: Attempting to Open com channel %s" % self.port)
                self.serial_port = serial.Serial(self.port, self.baudrate,timeout=self.timeout)
            except (serial.serialutil.SerialException), e:
                loginf("Failed to open serial port: %s" % e)
                loginf(traceback.format_exc())
                
                # Sometimes the arduino comes in on a different port. 
                if str(e).find("could not open port") != -1:
                    if str(e).find("No such file or directory") != -1:
                        loginf("Stoic: Attempting a different port")
                        
                        self.port = self.stoic_Cal_dict["serial_port_Prefix"] + str(PortNum)
                        PortNum += 1
                
                # on the last try give up completely, on the others take a nap
                if i == NumTrys - 1:
                    raise
                else:
                    time.sleep(5)
            else:
                break
        
        # python -m serial.tools.list_ports
        # Above will list ports

# TODO should we handle rain in some way here
    def close(self):
        loginf("Stoic: Close com channel >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" )
        if self.serial_port is not None:
            logdbg("close serial port %s" % self.port)
            self.serial_port.close()
            self.serial_port = None
            
    def get_raw_data(self):
        LineIn = self.serial_port.readline()
        if self._debug_serial:
            logdbg("StoicWS said: %s" % LineIn)
            
        return LineIn
    
    def sensor_parse_MCP9808_Temp(self, DataHex):
        """
        Parse the two hex bytes output from an MCP9808 tempriture sensor.
        Returns tempriture C
        """
        DataBin = int(DataHex,16)
        
        # Top three bits are garbage
        DataBin = DataBin & int("1FFF",16)
        
        #4th bit from highest is sign
        if((DataBin >> 12) == 0):
            Temp = float(DataBin & int("0FFF",16)) * (2**-4)
            
        else: # 1 and thus negative
            Temp = 256.0 - (float(DataBin & int("0FFF",16)) * (2**-4))
            
        return Temp
            
            
        
    
    def key_parse_2T_BoxTemp(self,LineIn):
        """
        Parse key 2T the temperature in the circuit box
        Should provide one value in 2 byte HEX eg
        1A5D
        
        entered as extraTemp1
        Units C
        """
        pos = LineIn.find(",")
        # Does the line have a checksum indicator
        if(LineIn.find("^") == -1):
            posEnd = LineIn.find(";")
        else:
            posEnd = LineIn.find(",")
        
        Temp = self.sensor_parse_MCP9808_Temp(LineIn[pos+1:posEnd])
        
        loginf("key_parse_2T_BoxTemp temp %f" % Temp)
            
        data = dict()
        data["extraTemp1"] = Temp
        
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
        
        #loginf("Stoic sensor_parse_BME280_Pressure  HEX in %s" % DataHex)
        
        RawPressure = long(DataHex,16) >> 4
        
        #loginf("Stoic sensor_parse_BME280_Pressure  RawPressure %d" % RawPressure)
        #loginf("Stoic sensor_parse_BME280_Pressure  RawPressure %X" % RawPressure)
        
        
        # TEST Line
        #loginf("self.stoic_Cal_dict[BME280ID+'_CAL_P6''] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_P6"])
        #loginf("self.stoic_Cal_dict[BME280ID+'_CAL_P6''] type  %s" % type(self.stoic_Cal_dict[BME280ID+"_CAL_P6"]))

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
        #loginf("Stoic sensor_parse_BME280_Pressure  Integer Pressure %d" % p)
        #loginf("Stoic sensor_parse_BME280_Pressure  Integer Pressure %X" % p)
        
        # Units of hPa or mbar (same thing)
        Pressure = float(p) / float(25600.0)

        loginf("Stoic sensor_parse_BME280_Pressure Pressure %f" % Pressure)

        return Pressure
        
            
    def sensor_parse_BME280_TFine(self, DataHex, BME280ID):
        """
        The BME280ID allows for mutiple BME280s on a single system
        
        The BME280 calibration produces a value called TFine which is used in later calibrations. 
        TFine is returned by this function.
        
        Input is 3 byte of HEX. Only 20 bits have meaning
        """
    
        
        RawTemp = long(DataHex,16) >> 4
        
        #loginf("Stoic sensor_parse_BME280_TFine  RawTemp %d" % RawTemp)
        #loginf("Stoic sensor_parse_BME280_TFine  RawTemp %X" % RawTemp)
        
        #loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T1'] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_T1"])
        #loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T2'] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_T2"])
        #loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T3'] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_T3"])
        
        #loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T1'] %X" % self.stoic_Cal_dict[BME280ID+"_CAL_T1"])
        #loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T2'] %X" % self.stoic_Cal_dict[BME280ID+"_CAL_T2"])
        #loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T3'] %X" % self.stoic_Cal_dict[BME280ID+"_CAL_T3"])
        
        # This mess comes from the data sheet. Someone must like LISP
        var1  = ( ((RawTemp>>3) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"]<<1)) * (self.stoic_Cal_dict[BME280ID+"_CAL_T2"]) ) >> 11
        #loginf("Stoic sensor_parse_BME280_TFine var1 %d" % var1)
        var2  = (( ( ((RawTemp>>4) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"])) * ((RawTemp>>4) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"])) ) >> 12) * (self.stoic_Cal_dict[BME280ID+"_CAL_T3"]) ) >> 14
        #loginf("Stoic sensor_parse_BME280_TFine var2 %d" % var2)
        TFine = var1 + var2
        
        #TEST Line
        #loginf("Stoic sensor_parse_BME280_TFine %d" % TFine)
       # loginf("Stoic sensor_parse_BME280_TFine type %s" % type(TFine))
         

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
        
        #loginf("Stoic sensor_parse_BME280_Temperature TFine %s" % type(TFine))
        
        #loginf("Stoic sensor_parse_BME280_Temperature TFine %d" % TFine)
        
       # loginf("(TFine * 5 ) >> 8 %d" % ((TFine * 5 ) >> 8))
        #loginf("float((TFine * 5 ) >> 8) %f" % float((TFine * 5 ) >> 8))
        #loginf("float((TFine * 5 ) >> 8)/float(100.0) %f" % (float((TFine * 5 ) >> 8)/float(100.0)))
        
        Temperature = float((TFine * 5 ) >> 8)/float(100.0)
        
        #TEST Line
        loginf("Stoic sensor_parse_BME280_Temperature %f " % Temperature)
        
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
        
        #TEST Line
        loginf("Stoic sensor_parse_BME280_Humidity %f " % Hum)

        return Hum
    
    
    def key_parse_3TPH_FARS(self,LineIn):
        """
        Data from BME280
        Line in follows 
        *3TPH,PPPPPP,TTTTTT,HHHH,^(Check bits may go here some day);
        *3TPH,5EC2E0,7E40D0,69A1,^;
        """
        
        BME280ID = "BME280_1"
        
        
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
            data["outTemp"] = None
            data["pressure"] = None
            data["outHumidity"] = None
            return data
        
        Temperature = self.sensor_parse_BME280_Temperature(TFine)
        
        Pressure = self.sensor_parse_BME280_Pressure(LineIn[PposStart+1:PposEnd+1],BME280ID, TFine)
            
        Humidity = self.sensor_parse_BME280_Humidity(LineIn[HposStart+1:HposEnd+1],BME280ID, TFine)
        
        data = dict()
        data["outTemp"] = Temperature
        data["pressure"] = Pressure
        data["outHumidity"] = Humidity
        return data
    
    def sensor_parse_TippingBuckedt_Rain(self,DataHexCurrent,DataHexLast):
        """
        Handles output from a tipping bucket rain gauge. Output is the difference between 
        current reat and last read.
        
        It is assumed that this will read out often enough that a max int of 15 will not be a problem.
        
        Units mm
        
        Usess rain_mm_Per_Tip in weewx.conf for converting bucket tips to mm
        """
        
        # TEST line
        #loginf("And in self %s" % self.stoic_Cal_dict["rain_mm_Per_Tip"])
        try:
            CurrentRainRaw = int(DataHexCurrent,16)
            LastRainRaw = int(DataHexLast,16)
        except (ValueError), e:
            loginf("Failed to parse or failed to get a proper hex number, DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
            logerr("Failed to parse or failed to get a proper hex number, DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
            loginf(traceback.format_exc())
            logerr(traceback.format_exc())
            raise
        
        # Test Line
        loginf("Sensor_parse_TippingBuckedt_Rain DataHexCurrent, DataHexLast: %s %s" %(DataHexCurrent, DataHexLast))
        
        # The counter max int is 15
        if(CurrentRainRaw < LastRainRaw):
            CurrentRainRaw += 16
        
        if(CurrentRainRaw == LastRainRaw):
            return 0.0
        # TODO does this work (float)0
        
        # TODO FIx this
        ConversionFactor = 1.0
                 
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
        
            Rain = self.sensor_parse_TippingBuckedt_Rain(LineIn[posStartCurrent+1:posEndCurrent+1],LineIn[posStartLast+1:posEndLast])
            
            data = dict()
            data["rain"] = Rain
            
        except:
            loginf("RAIN LOST Total failure in rain. Rain data may have been lost., LineIn: %s" %LineIn)
            logerr("RAIN LOST Total failure in rain. Rain data may have been lost., LineIn: %s" %LineIn)
            logmsg(LOG_EMERG, "STOIC: RAIN LOST Total failure in rain. Rain data may have been lost., LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            logerr(traceback.format_exc())
            raise
        
        
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
        
        loginf("sensor_parse_wind_direction_from_ADC WindADCRaw Model WindDirectionDeg %d %s, %f" % (WindADCRaw, Model, WindDirectionDeg))
        
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
            logdbg("StoicWS wind_gust_line_validation no *6WGS")
            return False
        pos = LineIn.find(";")
        if pos == -1:
            logdbg("StoicWS wind_gust_line_validation no ;")
            return False
        if LineIn[pos:].find("+5WGD") == -1:
            logdbg("StoicWS wind_gust_line_validation no +5WGD")
            return False
        if LineIn[pos+1:].find(";") == -1:
            logdbg("StoicWS wind_gust_line_validation no second ;")
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
        
        
        
        data = dict()
        data["windGustDir"] = GustDirection
        data["windGust"] = GustSpeed
        
        # TEST lines
        loginf("key_parse_6WGS_5WGD_wind_gust windGustDir %f" % data["windGustDir"])
        loginf("key_parse_6WGS_5WGD_wind_gust windGust %f" % data["windGust"])
        
        return data
    
    

    def sensor_parse_wind_direction_mean(self,LineInDirection,LineInBin):
        """
        Mean direction is tricky. The Stoic Watcher bins and data and adds a rotation. Then it sums the the rotated readings.
        
        Here we devide by the total number of readings and un rotate.
        """
        
        #Test Lines
        loginf("sensor_parse_wind_direction_mean LineInDirection %s" % LineInDirection)
        loginf("sensor_parse_wind_direction_mean LineInBin %s" % LineInBin)
        
        DirectionRaw = int(LineInDirection,16)
        BinNumber = int(LineInBin,16)
        
        loginf("sensor_parse_wind_direction_mean DirectionRaw %d" % DirectionRaw)
        loginf("sensor_parse_wind_direction_mean BinNumber %d" % BinNumber)
        
        # Divide to take mean
        DirectionMeanRaw = DirectionRaw / self.stoic_Cal_dict["wind_direction_number_of_mean_points"]
        
        # unrotate
        if(BinNumber >= (self.stoic_Cal_dict["wind_direction_number_of_bins"] / 2)):
            rotation = 1535 - ((BinNumber * 64) + self.stoic_Cal_dict["wind_direction_half_bin_size"])
        else:
            rotation = 511 - ((BinNumber * 64) + self.stoic_Cal_dict["wind_direction_half_bin_size"])
        
        
        loginf("DirectionMeanRaw not yet unrotated %d" % DirectionMeanRaw)
        
        DirectionMeanRaw = DirectionMeanRaw - rotation
        
        loginf("DirectionMeanRaw unrotated %d" % DirectionMeanRaw)
        
        if DirectionMeanRaw < 0:
            DirectionMeanRaw += 1024
        
        if DirectionMeanRaw > 1023:
            DirectionMeanRaw -= 1024
            
        loginf("sensor_parse_wind_direction_mean DirectionMeanRaw %d" % DirectionMeanRaw)
        loginf("sensor_parse_wind_direction_mean DirectionMeanRaw type %s" % type(DirectionMeanRaw))
       
        WindDirectionMean = self.sensor_parse_wind_direction_from_ADC(DirectionMeanRaw)
        
        if WindDirectionMean > 360:
            loginf("sensor_parse_wind_direction_mean this should never happen > 360")
            WindDirectionMean -= 360
            
        if WindDirectionMean < 0:
            loginf("sensor_parse_wind_direction_mean this should never happen < 0")
            WindDirectionMean += 360
        
        loginf("sensor_parse_wind_direction_mean WindDirectionMean %f" % WindDirectionMean)
        
        return WindDirectionMean
    
        
    def sensor_parse_wind_speed_mean(self,LineIn_6WMS):
        """
        The wind mean is a simple thing. The Stoic Watcher sends the difference between the current count and the count at time interval ago.
        V = P(2.25/T)
        For the davis anemometer, speed (mph) = counts * 2.25  / time of sample
        We report speed in m/s here
        
        """
        # Test line
        loginf("LineIn_6WMS %s" % LineIn_6WMS)
        
        MeanSpeedRaw = int(LineIn_6WMS,16)
        
        loginf("MeanSpeedRaw %d" % MeanSpeedRaw)
        
        MeanSpeed = float(MeanSpeedRaw) * self.stoic_Cal_dict["wind_mps_per_click_per_120_s"]
        
        loginf("MeanSpeed %f" % MeanSpeed)
        
        return MeanSpeed
        
    def wind_mean_line_validation(self,LineIn):
        if LineIn.find("*6WMS") == -1:
            logdbg("StoicWS wind_mean_line_validation no *6WMS")
            return False
        pos = LineIn.find(";")
        if pos == -1:
            logdbg("StoicWS wind_mean_line_validation no ;")
            return False
        if LineIn[pos:].find("+5WMD") == -1:
            logdbg("StoicWS wind_mean_line_validation no +5WMD")
            return False
        if LineIn[pos+1:].find(";") == -1:
            logdbg("StoicWS wind_mean_line_validation no second ;")
            return False
        
        
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
        
        loginf("key_parse_6WMS_5WMD_wind_mean LineIn %s" % LineIn)
        
        loginf("key_parse_6WMS_5WMD_wind_mean speed LineIn[posStart:posEnd] %s" % LineIn[posStart:posEnd])
        
        MeanSpeed = self.sensor_parse_wind_speed_mean(LineIn[posStart:posEnd])
        
        posSecondBlock = LineIn.find("+")
        
        posStart = LineIn[posSecondBlock+1:].find(",") + posSecondBlock +1 +1
        posMiddle = LineIn[posStart:].find(",") + posStart 
        posEnd = LineIn[posSecondBlock+1:].find(";") + posSecondBlock +1

        
        loginf("key_parse_6WMS_5WMD_wind_mean dir LineIn[posStart:posMiddle] %s" % LineIn[posStart:posMiddle])
        loginf("key_parse_6WMS_5WMD_wind_mean bin LineIn[posMiddle +1:posEnd] %s" % LineIn[posMiddle +1:posEnd])
        
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
        
        
        
        data = dict()
        data["windDir"] = MeanDirection
        data["windSpeed"] = MeanSpeed
        
        # TEST lines
        loginf("key_parse_6WMS_5WMD_wind_mean windDir %f" % data["windDir"])
        loginf("key_parse_6WMS_5WMD_wind_mean windSpeed %f" % data["windSpeed"])
        
        return data
    
        
        
    
    def parse_raw_data(self, LineIn):
        if LineIn[0] != "*":
            logdbg("StoicWS parse_raw_data given bad data")
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
            else:
            # TODO fix this
                return None
        except (ValueError), e:
            loginf("ValueError - parse_raw_data - Failed to get a proper hex number?, LineIn: %s" %LineIn)
            logerr("ValueError - parse_raw_data - Failed to get a proper hex number?, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (LookupError), e:
            loginf("LookupError - parse_raw_data - LineIn: %s" %LineIn)
            logerr("LookupError - parse_raw_data - LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (IndexError, KeyError), e:
            loginf("IndexError, KeyError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            logerr("IndexError, KeyError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (ArithmeticError), e:
            loginf("ArithmeticError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            logerr("ArithmeticError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        except (TypeError), e:
            loginf("TypeError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            logerr("TypeError - parse_raw_data - GOK, LineIn: %s" %LineIn)
            loginf(traceback.format_exc())
            return None
        
            
    
    @staticmethod
    def validate_string(LineIn):
        # TODO add aditional validation
        
        #loginf("Stoic: validate_string: %s" %LineIn)
        
        if (len(LineIn)<1):
            raise weewx.WeeWxIOError("Unexpected line length %d" % len(LineIn))
    
        # Rain cannot be easily recovered if lost. 
        if(LineIn.find("4R") != -1):
            if(LineIn.find("4R") != 1):
                loginf("RAIN LOST validate_string found '4R' out of place. Rain data may have been lost., LineIn: %s" %LineIn)
                logerr("RAIN LOST validate_string found '4R' out of place. Rain data may have been lost., LineIn: %s" %LineIn)
                logmsg(LOG_EMERG, "STOIC: RAIN LOST validate_string found '4R' out of place. Rain data may have been lost., LineIn: %s" %LineIn)
                # TODO attempt recovery by reading the rain
                raise weewx.WeeWxIOError(" '4R' is out of place. Rain data may have been lost %s" % LineIn)
        
        if(LineIn.find(";") == -1):
            raise weewx.WeeWxIOError("Line lacks terminator ; %s" % LineIn)
        if(LineIn[1:].find("*") != -1):
            raise weewx.WeeWxIOError("Line has extra start * %s" % LineIn)
        
        pos = LineIn[1:].find("+")
        if pos != -1:
            if(LineIn[pos:].find(";") == -1):
                raise weewx.WeeWxIOError("Line lacks second terminator ; %s" % LineIn)
        
        # Check line starts
        ValidLineStarts = set(["*","#","!"])
        if not (LineIn[0:1] in ValidLineStarts):
            raise weewx.WeeWxIOError("Line lacks valid line start *,#,! %s" % LineIn)
        
        #Only for data strings that start with data cue
        if(LineIn.find("*") == 0):
            if(LineIn.find(",") == -1):
                raise weewx.WeeWxIOError("Line lacks seporator , %s" % LineIn)
    
    
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
            self.send_reset_signal_to_Arduino()
            #TODO impliment the above better
            raise weewx.RetriesExceeded(msg)
        
    def get_processed_data_with_retry(self, max_tries=25, retry_wait=3):
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
                # Not a valid line, validate_string should eleminate these but simply ignored here
        
        ParsedData = self.parse_raw_data(LineIn)
        
        if ParsedData == None:
            loginf('StoicWS: Unable to parse %s' % LineIn)
        
        return ParsedData
    
    def send_reset_signal_to_Arduino(self):
        # send serial reset to Arduino
        # TODO impliment send_reset_signal_to_Arduino
        loginf("Attempting serial reset of Arduino")
        
        # Send Reset signal
        
        # Varify data received
        # raise exception if unable to reset
            
            
            
        




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
                      default=StoicWatcher.DEFAULT_PORT)
    (options, args) = parser.parse_args()

    if options.version:
        print "StoicWS driver version %s" % DRIVER_VERSION
        exit(0)

    with StoicWatcher(options.port, debug_serial=options.debug) as StoicWatcher:
        while True:
            print time.time(), _fmt(StoicWatcher.get_raw_data())
    
