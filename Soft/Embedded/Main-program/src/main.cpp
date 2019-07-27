#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "pins.h"
#include "MPU9250.h"
#include "main.h"

TwoWire	i2c_2 (2);
MPU9250 IMU(i2c_2, 0x68);

void setup()
{
	Serial.begin();
	delay(2000);
	Serial.println("Started");
	pinMode(BRIGHT_LED_PIN, OUTPUT);
	initMpu();
	Serial.println("Init completed");
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
	// setting the accelerometer full scale range to +/-8G
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