#include "BMP180.h"

#include "stdint.h"
#include "I2Cdev.h"

BMP180::BMP180()
{}

bool BMP180::begin()
{
	return (testConnection());
}

bool BMP180::testConnection  (void)
{
  uint8_t idRead;
  
  idRead = readByte(BMP180_REG_CHIP_ID_ADDRESS);
  return (CHIP_ID == idRead);
}

bool BMP180::startTemperature(void)
{
	return (writeByte(BMP180_REG_CONTROL_ADDRESS, BMP180_COMMAND_TEMPERATURE));
}


uint8_t* BMP180::getTemperature(void)
{
	uint8_t temp[TEMP_SIZE];

  Fastwire::readBuf(BMP180_ADDR << 1, BMP180_REG_RESULT_ADDRESS, &temp[0], TEMP_SIZE);
  return(temp);
}


bool BMP180::startPressure(void)
{
	return (writeByte(BMP180_REG_CONTROL_ADDRESS, BMP180_COMMAND_PRESSURE0));
}


uint8_t* BMP180::getPressure(void)
{
	uint8_t pressure[PRESSURE_SIZE];

  Fastwire::readBuf(BMP180_ADDR << 1, BMP180_REG_RESULT_ADDRESS, &pressure[0], PRESSURE_SIZE);
	return(pressure);
}

bool BMP180::checkDataReady  (void)
{
  return !(readByte(BMP180_REG_CONTROL_ADDRESS) & DATA_READY_MASK);
}

uint8_t BMP180::readByte(uint8_t address)
{
  uint8_t buffer;
  
  I2Cdev::readByte(BMP180_ADDR, address, &buffer);
  return (buffer);
}

bool BMP180::writeByte(uint8_t address, uint8_t data)
{
  bool success;

  success = I2Cdev::writeByte(BMP180_ADDR, address, data);
  return (success);
}
