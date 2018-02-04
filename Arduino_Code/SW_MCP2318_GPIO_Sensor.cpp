/*
 * SW_MCP2318_GPIO_Sensor.cpp
 *
 *  Created on: 2018-02-01
 *      Author: StoicWeather
 *
 */

/*
 * NWS Wind gust rapid fluctuations of wind speed with a variation of 10 knots or more between peaks and lulls.
 * The gust speed is the max speed recorded.
 */

#include "SW_MCP2318_GPIO_Sensor.h"

SW_MCP2318_GPIO_Sensor::SW_MCP2318_GPIO_Sensor(byte AddressIn, I2C I2CBussIn, byte NumberOfRecordsIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{

	// Mean Wind Measurements
	HaveFullQueue = false;
	CurrentQueueLoc = -1;
	// Counts by data points. With 13 bits it takes 2 bytes for each data point unless you want to get tricky.
	WindSpeedQueue = (byte*)calloc(NumberOfRecordsIn*2, sizeof(byte));
	QueueLength = NumberOfRecordsIn;

	// Gust measurements
	LastGustReadout = 0;
	HaveFullGustQueue = false;
	CurrentGustQueueLoc = -1;

	GustQueue = (byte*)calloc((int)NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP, sizeof(byte));




	Serial.print(F("#Wind Speed Initialized;"));



}

int SW_MCP2318_GPIO_Sensor::AcquireDataAndReturn()
{
	//SEQOP = 0 So the second byte should be GPIOA :)
	byte status = I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOB_REG, (byte)1);

	//TEST line
	/*Serial.print(F("# MCP2318 read status "));
	Serial.print(status,HEX);
	Serial.println(F(";"));*/

	//TEST line
					/*Serial.print(F("# MCP2318 I2c.available()  "));
					Serial.print(I2CBuss.available(),HEX);
					Serial.println(F(";"));*/
	//TEST line
	Serial.print(F("# MCP2318 readin "));

	int DataIn = (int)I2CBuss.receive();

	//TEST line
	Serial.print(DataIn, HEX);

	I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOA_REG, (byte)1);

	// Most sig bits came in first. They get shifted up. Only 13 bits of data.
	DataIn = DataIn &0b00011111;
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();
	//TEST line
	Serial.print(DataIn, HEX);
	Serial.println(F(";"));

	//TEST LINE
	Serial.print(F("# MCP2318 readin "));
	Serial.print(DataIn, HEX);
	Serial.println(F(";"));

	return DataIn;
}

bool SW_MCP2318_GPIO_Sensor::AcquireData()
{
	// Prep queue
	CurrentQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >=?
	if(CurrentQueueLoc >= QueueLength)
	{
		CurrentQueueLoc = 0;
		HaveFullQueue = true;
	}

	/*//SEQOP = 0 So the second byte should be GPIOA :)
	byte status = I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOB_REG, (byte)2);

	//TEST line
	Serial.print(F("# MCP2318 read status "));
	Serial.print(status);
	Serial.println(F(";"));

	int DataIn = (int)I2CBuss.receive();
	// Most sig bits came in first. They get shifted up. Only 13 bits of data.
	DataIn = DataIn &0b00011111;
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();

	//TEST LINE
	Serial.print(F("# MCP2318 readin "));
		Serial.print(DataIn, HEX);
		Serial.println(F(";"));
	 */
	int DataIn = AcquireDataAndReturn();

	WindSpeedQueue[CurrentQueueLoc*2] = (byte)(DataIn >> 8);

	// (byte) should truncate
	WindSpeedQueue[(CurrentQueueLoc*2)+1] = (byte)(DataIn);



	return true;



}

bool SW_MCP2318_GPIO_Sensor::SendAllRawDataSerial()
{

	if(!HaveFullQueue)
	{

		return false;
	}

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("WA,"));
	byte QueueLoc = 0;
	for(int i = 0; i< QueueLength; i++)
	{
		QueueLoc = CurrentQueueLoc + i;

		// TODO CurrentQueueLoc >QueueLength >=?
		if(QueueLoc >= QueueLength)
		{
			QueueLoc = 0;
		}

		SerialHexBytePrint(WindSpeedQueue[QueueLoc*2]);
		SerialHexBytePrint(WindSpeedQueue[(QueueLoc*2)+1]);
	}

	Serial.print(F(";"));

	return true;



}

// todo fix this
bool SW_MCP2318_GPIO_Sensor::SendRawDataSerial()
{
	return SendAllRawDataSerial();
}

// Returns counts. Divide byt
int SW_MCP2318_GPIO_Sensor::GetMostRecentRawMean()
{
	if(!HaveFullQueue)
	{
		return 0;
	}

	byte OldestQueueLoc = 0;
	if(CurrentQueueLoc != QueueLength - 1)
	{
		OldestQueueLoc = CurrentQueueLoc + 1;
	}
	else
	{
		OldestQueueLoc = 0;
	}

	int Difference = ((WindSpeedQueue[CurrentQueueLoc*2] << 8) + WindSpeedQueue[(CurrentQueueLoc*2)+1])
					- ((WindSpeedQueue[OldestQueueLoc*2] << 8) + WindSpeedQueue[(OldestQueueLoc*2)+1]);

	if(Difference < 0)
	{
		Difference += MAX_WIND_CTS;
	}

	return Difference;

}

bool SW_MCP2318_GPIO_Sensor::SendMostRecentRawMean()
{
	if(!HaveFullQueue)
	{
		return false;
	}

	int Difference = GetMostRecentRawMean();

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("WM,"));
	SerialHexBytePrint((byte)(Difference >> 8));
	SerialHexBytePrint((byte)Difference);
	Serial.print(F(";"));


	return true;

}

// A gust is 10 knts between peak and lull in a time period that is rapid.
//Readout is every 2.25 seconds for instantaneous speed.
// How many 2.25 second intervals is set by gust Record Interval
//bool SW_MCP2318_GPIO_Sensor::AcquireGustData();
// THis sends raw counts from a 2.25 second period. Sends max, min, and current for
// Time period recorded.
//bool SW_MCP2318_GPIO_Sensor::SendGustData();

bool SW_MCP2318_GPIO_Sensor::AcquireGustDataAndSend()
{


	int CurrentGustReadout = AcquireDataAndReturn();

	//Very first run just record it and be done
	if(CurrentGustQueueLoc == -1)
	{
		CurrentGustQueueLoc++;
		GustQueue[CurrentGustQueueLoc] = byte(CurrentGustReadout);
		return false;
	}

	//  Using differences to keep memory to a minimum. Largest readout in 2.25 seconds is 100
	// The readout may wrap around.
	byte CurrentGustRaw;
	if(LastGustReadout > CurrentGustReadout)
	{
		CurrentGustRaw = (byte)(CurrentGustReadout + (int)MAX_WIND_CTS - LastGustReadout);
	}
	else
	{
		CurrentGustRaw = (byte)(CurrentGustReadout - LastGustReadout);
	}

	// Used next time
	LastGustReadout = CurrentGustReadout;

	// Reporting only done with a full queue
	if(HaveFullGustQueue)
	{
		// Search for min and max. Most we should ever get is 100 or less
		byte min = 150;
		byte max = 0;
		for(int i = 0; i < (int)NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP; i++)
		{
			if(GustQueue[i] < min)
			{
				min = GustQueue[i];
			}
			if(GustQueue[i] > max)
			{
				max = GustQueue[i];
			}
		}

		// Report current, min and max
		Serial.print(F("*"));
		Serial.print(SensorNumber,DEC);
		Serial.print(F("WG,"));
		SerialHexBytePrint(CurrentGustRaw);
		Serial.print(F(","));
		SerialHexBytePrint(min);
		Serial.print(F(","));
		SerialHexBytePrint(max);
		Serial.println(F(";"));

	} // end if HaveFullGustQueue

	// Add current to queue
	CurrentGustQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >=?
	if(CurrentGustQueueLoc >= (int)NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP)
	{
		CurrentGustQueueLoc = 0;
		HaveFullGustQueue = true;
	}

	GustQueue[CurrentGustQueueLoc] = CurrentGustRaw;

	Serial.print(F("# Current wind gust raw "));
	Serial.print(CurrentGustRaw,HEX);
	Serial.println(F(";"));


	return true;

}

bool SW_MCP2318_GPIO_Sensor::InitializeSensor()
{
	Serial.println(F("#Initializing MCP2318;"));

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/

	// Set configuration parameters
	// Only needed if we change from defaults
	//I2CBuss.write(SensorAddress, (byte)MCP2318_IOCON_REG, (byte)MCP2318_CONFIG_IOCON_CMD);

	// Only need to set IODIR if using output.



	return true;

}



#ifdef VERIFY_CHIPS
bool SW_MCP2318_GPIO_Sensor::VerifyChip()
{
	// There are no IDs baked in. We verify the default startup conditions

	// Check IOCON default
	byte status = I2CBuss.read(SensorAddress, (byte)MCP2318_IOCON_REG, (byte)2);

	//TEST line
				Serial.print(F("# MCP2318 I2c.available()  "));
				Serial.print(I2CBuss.available(),HEX);
				Serial.println(F(";"));

	int DataIn = (int)I2CBuss.receive();
	// Most sig bits came in first. They get shifted up.
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();

	//TEST line
	Serial.print(F("# MCP23018 VerifyChip read status "));
	Serial.print(status,HEX);
	Serial.println(F(";"));


	if(DataIn == (int)MCP2318_IOCON_DEFAULT_VALUE)
	{
		Serial.println(F("#MCP23018 Correct IOCON Values;"));
	}
	else
	{
		Serial.println(F("#NOT MCP23018 Correct IOCON Values;"));
		return false;
	}

	// Check IODIR default
	status = I2CBuss.read(SensorAddress, (byte)MCP2318_IODIRA_REG, (byte)2);


		//TEST line
			Serial.print(F("# MCP23018 I2c.available()  "));
			Serial.print(I2CBuss.available(),HEX);
			Serial.println(F(";"));

	DataIn = (int)I2CBuss.receive();
	// Most sig bits came in first. They get shifted up.
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();

	//TEST line
	Serial.print(F("# MCP2318 VerifyChip read status "));
	Serial.print(status,HEX);
	Serial.println(F(";"));


	if(DataIn == (int)MCP2318_IODIRA_DEFAULT_VALUE)
	{
		Serial.println(F("#MCP2318 Correct IODIR Values;"));
	}
	else
	{
		Serial.println(F("#NOT MCP2318 Correct IODIR Values;"));
		Serial.println(DataIn);
		return false;
	}


	return true;

}
#endif /*VERIFY_CHIPS*/
