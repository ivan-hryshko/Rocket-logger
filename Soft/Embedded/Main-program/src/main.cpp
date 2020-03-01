#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "pins.h"
#include "MPU9250.h"
#include "main.h"
#include <SdioF1.h>

TwoWire i2c_2(2);
MPU9250 IMU(i2c_2, 0x68);

SdFatSdio sd;

// Size of read/write.
const size_t BUF_SIZE = 512;

// File size in MB where MB = 1,000,000 bytes.
const uint32_t FILE_SIZE_MB = 5;

// Write pass count.
const uint8_t WRITE_COUNT = 2;

// Read pass count.
const uint8_t READ_COUNT = 2;
//==============================================================================
// End of configuration constants.
//------------------------------------------------------------------------------
// File size in bytes.
const uint32_t FILE_SIZE = 1000000UL * FILE_SIZE_MB;

uint8_t buf[BUF_SIZE];

SdFile file;

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(F(s))
//------------------------------------------------------------------------------
void cidDmp()
{
	cid_t cid;
	if (!sd.card()->readCID(&cid))
	{
		error("readCID failed");
	}
	cout << F("\nManufacturer ID: ");
	cout << hex << int(cid.mid) << dec << endl;
	cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
	cout << F("Product: ");
	for (uint8_t i = 0; i < 5; i++)
	{
		cout << cid.pnm[i];
	}
	cout << F("\nVersion: ");
	cout << int(cid.prv_n) << '.' << int(cid.prv_m) << endl;
	cout << F("Serial number: ") << hex << cid.psn << dec << endl;
	cout << F("Manufacturing date: ");
	cout << int(cid.mdt_month) << '/';
	cout << (2000 + cid.mdt_year_low + 10 * cid.mdt_year_high) << endl;
	cout << endl;
}

//void yield() {}

void sdLoop(void)
{
	float s;
	uint32_t t;
	uint32_t maxLatency;
	uint32_t minLatency;
	uint32_t totalLatency;

	cout << F("Type any character to start\n");
	while (!Serial.available())
	{
		SysCall::yield();
	}
	//cout << F("chipSelect: ") << int(chipSelect) << endl;
	//cout << F("FreeStack: ") << FreeStack() << endl;

	if (sd.begin())
	{
		cout << F("Type is FAT") << int(sd.vol()->fatType()) << endl;
		cout << F("Card size: ") << ((float)(sd.card()->cardSize())/(1<<30)) << "GB" << endl;

		cidDmp();

		// open or create file - truncate existing file.
		if (!file.open("bench.dat", O_RDWR | O_CREAT | O_TRUNC))
		{
			error("open failed");
		}

		// fill buf with known data
		for (size_t i = 0; i < (BUF_SIZE - 2); i++)
		{
			buf[i] = 'A' + (i % 26);
		}
		buf[BUF_SIZE - 2] = '\r';
		buf[BUF_SIZE - 1] = '\n';

		cout << F("File size ") << FILE_SIZE_MB << F(" MB\n");
		cout << F("Buffer size ") << BUF_SIZE << F(" bytes\n");
		cout << F("Starting write test, please wait.") << endl
			 << endl;

		// do write test
		uint32_t n = FILE_SIZE / sizeof(buf);
		cout << F("write speed and latency") << endl;
		cout << F("speed,max,min,avg") << endl;
		cout << F("KB/Sec,usec,usec,usec") << endl;
		for (uint8_t nTest = 0; nTest < WRITE_COUNT; nTest++)
		{
			file.truncate(0);
			maxLatency = 0;
			minLatency = 9999999;
			totalLatency = 0;
			t = millis();
			for (uint32_t i = 0; i < n; i++)
			{
				uint32_t m = micros();
				if (file.write(buf, sizeof(buf)) != sizeof(buf))
				{
					sd.errorPrint("write failed");
					file.close();
					return;
				}
				m = micros() - m;
				if (maxLatency < m)
				{
					maxLatency = m;
				}
				if (minLatency > m)
				{
					minLatency = m;
				}
				totalLatency += m;
			}
			file.sync();
			t = millis() - t;
			s = file.fileSize();
			cout << s / t << ',' << maxLatency << ',' << minLatency;
			cout << ',' << totalLatency / n << endl;
		}
		cout << endl
			 << F("Starting read test, please wait.") << endl;
		cout << endl
			 << F("read speed and latency") << endl;
		cout << F("speed,max,min,avg") << endl;
		cout << F("KB/Sec,usec,usec,usec") << endl;

		// do read test
		for (uint8_t nTest = 0; nTest < READ_COUNT; nTest++)
		{
			file.rewind();
			maxLatency = 0;
			minLatency = 9999999;
			totalLatency = 0;
			t = millis();
			for (uint32_t i = 0; i < n; i++)
			{
				buf[BUF_SIZE - 1] = 0;
				uint32_t m = micros();
				int32_t nr = file.read(buf, sizeof(buf));
				if (nr != sizeof(buf))
				{
					sd.errorPrint("read failed");
					file.close();
					return;
				}
				m = micros() - m;
				if (maxLatency < m)
				{
					maxLatency = m;
				}
				if (minLatency > m)
				{
					minLatency = m;
				}
				totalLatency += m;
				if (buf[BUF_SIZE - 1] != '\n')
				{
					error("data check");
				}
			}
			s = file.fileSize();
			t = millis() - t;
			cout << s / t << ',' << maxLatency << ',' << minLatency;
			cout << ',' << totalLatency / n << endl;
		}
		cout << endl
			 << F("Done") << endl;
		file.close();
	}
	else
	{
		Serial.println("SD init failed");
		sd.initErrorHalt();
	}
}

void setup()
{
	Serial.begin(115200);
	delay(5000);
	Serial.println("Started");
	pinMode(BRIGHT_LED_PIN, OUTPUT);
	initMpu();
	Serial.println("Init completed");
	//sdio_begin();
}

void loop()
{
	/*
	digitalWrite(BRIGHT_LED_PIN, HIGH);
	delay(2000);
	digitalWrite(BRIGHT_LED_PIN, LOW);
	delay(2000);
	*/
	readMpu();
	sdLoop();
}

bool initMpu(void)
{
	int status;

	Serial.print("MPU init ");
	status = IMU.begin();
	if (status < 0)
	{
		Serial.println("unsuccessful");
		Serial.println("Check IMU wiring or try cycling power");
		Serial.print("Status: ");
		Serial.println(status);
		return (false);
	}
	Serial.println("OK");
	// setting the accelerometer full scale range to +/-16G
	IMU.setAccelRange(MPU9250::ACCEL_RANGE_16G);
	// setting the gyroscope full scale range to +/-500 deg/s
	IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
	// setting DLPF bandwidth to 20 Hz
	IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
	// setting SRD to 19 for a 50 Hz update rate
	IMU.setSrd(19);
	Serial.println("OK");

	return (true);
}

void readMpu(void)
{
	IMU.readSensor();

	// display the data
	Serial.print(IMU.getAccelX_mss(), 6);
	Serial.print("\t");
	Serial.print(IMU.getAccelY_mss(), 6);
	Serial.print("\t");
	Serial.print(IMU.getAccelZ_mss(), 6);
	Serial.print("\t");
	Serial.print(IMU.getGyroX_rads(), 6);
	Serial.print("\t");
	Serial.print(IMU.getGyroY_rads(), 6);
	Serial.print("\t");
	Serial.print(IMU.getGyroZ_rads(), 6);
	Serial.print("\t");
	//Serial.print(IMU.getMagX_uT(), 6);
	//Serial.print("\t");
	//Serial.print(IMU.getMagY_uT(), 6);
	//Serial.print("\t");
	//Serial.print(IMU.getMagZ_uT(), 6);
	//Serial.print("\t");
	Serial.println(IMU.getTemperature_C(), 6);
	delay(20);
}