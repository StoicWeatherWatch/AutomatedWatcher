#!/usr/bin/env python
#
# Stoic WS
# Version 0.0.4
# 2018-01-27
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

Supported Keys:                                    Mapped DB name
2T Temperature in Box  -                           extraTemp1
3TPH Temperature, pressure, humidity in FARS -     outTemp, pressure, outHumidity
   This is BME280ID "BME280-1"
4R Rain tipping bucket - rain
   Reported since last report - a difference

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


# TODO DO I need this? Python 2.7 should have with_statement
from __future__ import with_statement

import serial
import syslog
import time
import binascii

import weewx.drivers

DRIVER_NAME = 'StoicWS'
DRIVER_VERSION = '0.0.4'

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
        
        # Rain
        stoic_Cal_dict["rain_mm_Per_Tip"] = float(stn_dict.get('rain_mm_Per_Tip'))
        
        # BME280 A 3TPH "BME280-1"
        # The BME280 has lots of calibration data which is hard coded in the factory. 
        # The calculations require at least 64 bit integer math
        # Not easily automated at read in either...
        #  Data sheet T1 Unsigned 
        #stoic_Cal_dict["BME280_1_CAL_T1"] = (long(stn_dict.get("cal-BME280-1.2.1"),16) << 8) + long(stn_dict.get("cal-BME280-1.2.0"),16)
        
        # TEST Lines
        loginf("stn_dict.get('cal-BME280-1.2.1') %s" % stn_dict.get("cal-BME280-1.2.1"))
        loginf("stn_dict.get('cal-BME280-1.2.0') %s" % stn_dict.get("cal-BME280-1.2.0"))
        
        stoic_Cal_dict["BME280_1_CAL_T1"] = BoschHEXHEX2UnsignedLong(stn_dict.get("cal-BME280-1.2.1"),stn_dict.get("cal-BME280-1.2.0"))
        # TEST LINE
        loginf("stoic_Cal_dict['BME280_1_CAL_T1'] %d" % stoic_Cal_dict["BME280_1_CAL_T1"])
        #  Data sheet T2 signed 
        #stoic_Cal_dict["BME280_1_CAL_T2"] = ((long(stn_dict.get("cal-BME280-1.2.3"),16) >> 7) * long(-1) ) * (((long(stn_dict.get("cal-BME280-1.2.3"),16) & 0b01111111) << 8) + long(stn_dict.get("cal-BME280-1.2.2"),16))
        stoic_Cal_dict["BME280_1_CAL_T2"] = BoschHEXHEX2SignedLong(stn_dict.get("cal-BME280-1.2.3"),stn_dict.get("cal-BME280-1.2.2"))
         # TEST LINE
        loginf("stoic_Cal_dict['BME280_1_CAL_T2'] %d" % stoic_Cal_dict["BME280_1_CAL_T2"])
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
        stoic_Cal_dict["BME280_1_CAL_H5"] = (long(sign) * (((long(stn_dict.get("cal-BME280-1.2.5"),16) & 0b01111111) << 4) + (long(stn_dict.get("cal-BME280-1.2.4"),16) >> 4) ))
        # TEST LINE
        loginf('stn_dict.get("cal-BME280-1.2.5") %s' % stn_dict.get("cal-BME280-1.2.5") )
        loginf('stn_dict.get("cal-BME280-1.2.4") %s' % stn_dict.get("cal-BME280-1.2.4") )
        loginf('long(stn_dict.get("cal-BME280-1.2.5"),16) %d' % long(stn_dict.get("cal-BME280-1.2.5"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.4"),16) %d' % long(stn_dict.get("cal-BME280-1.2.4"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.5"),16) %X' % long(stn_dict.get("cal-BME280-1.2.5"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.4"),16) %X' % long(stn_dict.get("cal-BME280-1.2.4"),16) )
        loginf('long(stn_dict.get("cal-BME280-1.2.5"),16) << 4 %d' % (long(stn_dict.get("cal-BME280-1.2.5"),16) << 4))
        loginf('((long(stn_dict.get("cal-BME280-1.2.5"),16) & 0b01111111) << 4) %X' % ((long(stn_dict.get("cal-BME280-1.2.5"),16) & 0b01111111) << 4))
        loginf('(long(stn_dict.get("cal-BME280-1.2.4"),16) >> 4) %X' % (long(stn_dict.get("cal-BME280-1.2.4"),16) >> 4))
        loginf("stoic_Cal_dict['BME280_1_CAL_H5'] %d" % stoic_Cal_dict["BME280_1_CAL_H5"])
        
        # TODO H5 still not correct
        
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
        self.port = stn_dict.get('port', StoicWatcher.DEFAULT_PORT)
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
            
        data = dict()
        data["extraTemp1"] = Temp
        
        return data
    
    def sensor_parse_BME280_Pressure(self, DataHex, BME280ID):
        """
        The BME280ID allows for mutiple BME280s on a single system with seporate calibrations.
        
        Pressure arrives as 3 hex bytes.
        Byte struction MSB xxxx xxxx LSB xxxx xxxx XLSB xxxx 0000
        """
        # TODO build
        return None
        
            
    def sensor_parse_BME280_TFine(self, DataHex, BME280ID):
        """
        The BME280ID allows for mutiple BME280s on a single system
        
        The BME280 calibration produces a value called TFine which is used in later calibrations. 
        TFine is returned by this function.
        
        Input is 3 byte of HEX. Only 20 bits have meaning
        """
        
        loginf("Stoic sensor_parse_BME280_TFine  HEX in %s" % DataHex)
        
        RawTemp = long(DataHex,16) >> 4
        
        loginf("Stoic sensor_parse_BME280_TFine  RawTemp %d" % RawTemp)
        loginf("Stoic sensor_parse_BME280_TFine  RawTemp %X" % RawTemp)
        
        loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T1'] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_T1"])
        loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T2'] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_T2"])
        loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T3'] %d" % self.stoic_Cal_dict[BME280ID+"_CAL_T3"])
        
        loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T1'] %X" % self.stoic_Cal_dict[BME280ID+"_CAL_T1"])
        loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T2'] %X" % self.stoic_Cal_dict[BME280ID+"_CAL_T2"])
        loginf("Stoic sensor_parse_BME280_TFine self.stoic_Cal_dict[BME280ID+'_CAL_T3'] %X" % self.stoic_Cal_dict[BME280ID+"_CAL_T3"])
        
        # This mess comes from the data sheet. Someone must like LISP
        var1  = ( ((RawTemp>>3) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"]<<1)) * (self.stoic_Cal_dict[BME280ID+"_CAL_T2"]) ) >> 11
        loginf("Stoic sensor_parse_BME280_TFine var1 %d" % var1)
        var2  = (( ( ((RawTemp>>4) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"])) * ((RawTemp>>4) - (self.stoic_Cal_dict[BME280ID+"_CAL_T1"])) ) >> 12) * (self.stoic_Cal_dict[BME280ID+"_CAL_T3"]) ) >> 14
        loginf("Stoic sensor_parse_BME280_TFine var2 %d" % var2)
        TFine = var1 + var2
        
        #TEST Line
        loginf("Stoic sensor_parse_BME280_TFine %d" % TFine)
         

        # The limits for the sensor are -40 to 85 C. Equivalent to an output of -4000 or 8500
        if (TFine < -204826):
            loginf("Stoic sensor_parse_BME280_TFine T less than min. Reporting None")
            return None
        elif (TFine > 435174):
            loginf("Stoic sensor_parse_BME280_TFine T greater than max. Reporting None")
            return None

        return TFine
    
    def sensor_parse_BME280_Temperature(self, TFine):
        if TFine == None:
            return None
        
        loginf("Stoic sensor_parse_BME280_Temperature TFine %s" % type(TFine))
        
        loginf("Stoic sensor_parse_BME280_Temperature TFine %d" % TFine)
        
        loginf("(TFine * 5 ) >> 8 %d" % ((TFine * 5 ) >> 8))
        loginf("float((TFine * 5 ) >> 8) %f" % float((TFine * 5 ) >> 8))
        loginf("float((TFine * 5 ) >> 8)/float(100.0) %f" % (float((TFine * 5 ) >> 8)/float(100.0)))
        
        Temperature = float((TFine * 5 ) >> 8)/float(100.0)
        
        #TEST Line
        loginf("Stoic sensor_parse_BME280_Temperature %f" % Temperature)
        
        return Temperature
            
    def sensor_parse_BME280_Humidity(self, DataHex, BME280ID):
        """
        The BME280ID allows for mutiple BME280s on a single system
        """
        # TODO build
        return 50
            
    
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
        
        Pressure = self.sensor_parse_BME280_Pressure(LineIn[PposStart+1:PposEnd+1],BME280ID)
            
        Humidity = self.sensor_parse_BME280_Humidity(LineIn[HposStart+1:HposEnd+1],BME280ID)
        
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
        
        CurrentRainRaw = int(DataHexCurrent,16)
        LastRainRaw = int(DataHexLast,16)
        
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
        
        Rain = self.sensor_parse_TippingBuckedt_Rain(LineIn[posStartCurrent+1:posEndCurrent+1],LineIn[posStartLast+1:posEndLast])
            
        data = dict()
        data["rain"] = Rain
        
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
            return self.key_parse_2T_BoxTemp(LineIn)
        elif LineIn[1:pos] == "3TPH":
            return self.key_parse_3TPH_FARS(LineIn)
        elif LineIn[1:pos] == "4R":
            return self.key_parse_4R_Rain(LineIn)

        else:
            # TODO fix this
            data = dict()
            data["inTemp"] = 15
            return data
    
    @staticmethod
    def validate_string(LineIn):
        # TODO add aditional validation
        if (len(LineIn)<1):
            raise weewx.WeeWxIOError("Unexpected line length %d" % len(LineIn))
        if(LineIn.find(";") == -1):
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
                # Not a valid line, Ignored
        
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
    
