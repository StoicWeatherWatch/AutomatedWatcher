# Save Serial
# Version 0.0.3
#JWW
# 17-12-24

# This opens a serial connection to an Arduino, reads data from it, and writes to a record file.
# Input format from Arduino
    # ! An alert in computer readable

    # * Data starts

    # ; end of data or alert line

    # # Comments ignored except in raw log files

    #   Arduino data line
    #   *DataID,Value;

    # CUrrently supported DataID
    # DataID: (Data ID is a single letter or number. Case sensitive)
    # L   Lux data from TSL2561 Source 1, Sensor 1, Measurement 1, Unit LUX

# Output Format
    # ear, Month, Day, Hour, Min, Sec, Source, Sensor, Measurement, Human readable Measurement, Unit, Value, <Future additions>,;

    # Time is UTC from GPS if possible with leading zeros

    # Source:
    # 1 Arduino
    # 2 Chip

    # Sensor:
    # 1 Lux light sensor

    # Measurement: (Some sensors have multiple measurements)
    # 1 Light level

    # Human readable Measurement:
    # Lit

    # Unit:
    # LUX

    # Value:
    # Decimal number


import time

import datetime

import serial
# Run this: pip install pyserial
# For pycharm see installing packages

def WriteHeaders(BlankFile):
    BlankFile.write(datetime.datetime.utcnow().strftime("# %y-%m-%d-%H-%M  UTC\n"))
    BlankFile.write(datetime.datetime.now().strftime("# %y-%m-%d-%H-%M  Local\n"))
    BlankFile.write("# Serial Output Recording\n")

def NewHourNewFile(OldFile):
    OldFile.close()
    NameDateString = datetime.datetime.utcnow().strftime("%y-%m-%d-%H_SerialOutput.txt")
    NewFile = open(NameDateString, "a")

    WriteHeaders(NewFile)

    return NewFile

#Strips comments and other crap
def ArduinoLineParser(InputLineString):
    #if InputLineString[0] == "#":
    #    return ""
    DataStartIndex = InputLineString.find("*")
    if DataStartIndex != -1:
        DataEndIndex = InputLineString.find(";")
        if DataEndIndex != -1:
            # Test both begining and end to skip lines where there may be errors or half write
            return InputLineString[DataStartIndex+1:DataEndIndex]
        else:
            print("Error no end ;")
            return ""
    else:
        # No data so return empty string
        return ""

# empty string evalutates to false myString == "" if mystring: -> False


def GetDate():
    # Not the romantic kind. Attempting to date your weather station is not recommended.
    # Psychrometers make lousy romantic partners. Always swinging around.

    # Test to see if GPS is available.
    gps = False
    if gps:
        # Get date from GPS
        print("Wow a GPS")
    else:
        CurrentDate = datetime.datetime.utcnow()

    return CurrentDate

# Turns the output from Arduino into
#Year, Month, Day, Hour, Min, Sec, Source, Sensor, Measurement, Unit, Value, <Future additions>, ;
def DataStringConverter(InputDataString):

    if InputDataString[0] == "L":
        # L   Lux data from TSL2561 Source 1, Sensor 1, Measurement 1,Humumen Mea Lit, Unit LUX
        Outstring = GetDate().strftime("%y,%m,%d,%H,%M,%S,") + "1,1,1,Lit,LUX," + InputDataString[2:] + ",;\n"

    #print(Outstring)
    #print(type(Outstring))

    return Outstring


#with open(sys.argv[3], "w") as f:
#  with serial.Serial(port=sys.argv[1], baudrate=sys.argv[2]) as ser:
 #   if ser.isOpen():
#     ser.readline()
 #   while ser.isOpen():
#      f.write('{}, {}'.format(datetime.datetime.now().strftime('%c'), ser.readline()))


# Locate the Arduino

# Open the serial connection and save to disk

# initial Setup - Make file name

NameDateString = datetime.datetime.utcnow().strftime("%y-%m-%d-%H_SerialOutput.txt")

# This is an int
CurrentHourUTC = datetime.datetime.utcnow().hour


OutFile = open(NameDateString, "a")
WriteHeaders(OutFile)

MacSerialPort = "/dev/cu.usbmodem14231"
ChipSerialPort = "/dev/ttyACM1"

ArduinoSerialConnection = serial.Serial(ChipSerialPort, 9600, timeout=2)
# python -m serial.tools.list_ports
# Above will list ports

# while loop in perpetuity
while True:
    # New file every hour
    if CurrentHourUTC != datetime.datetime.utcnow().hour:
        OutFile = NewHourNewFile(OutFile)

    #is Serial prot open
    #print("Need to put this test back in")
    #if ArduinoSerialConnection.is_open:
    if True:
            inString = ArduinoSerialConnection.readline().decode("utf-8", "backslashreplace")
            # Serial seems to spit out a bytes type string
            #print(inString)

            DataString = ArduinoLineParser(inString)
            if DataString:
                #Year, Month, Day, Hour, Min, Sec, Source, Sensor, Measurement, Unit, Value, ;
                OutFile.write(DataStringConverter(DataString))
                #print(DataString)

    else:
            print("False")
            OutFile.write("# Failed to open serial")
            OutFile.write(datetime.datetime.utcnow().strftime("# %y-%m-%d-%H-%M  UTC\n"))
    # Read the Serial port


OutFile.close()





# write data to File

