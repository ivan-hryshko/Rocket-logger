#ifndef BMP180_h
#define BMP180_h
#include "stdint.h"

class BMP180
{
	public:
		BMP180();

		bool begin();
    
		bool startTemperature  (void);
		uint8_t* getTemperature  (void);
    
		bool startPressure (void);
		uint8_t* getPressure  (void);

    bool checkDataReady  (void);
    bool testConnection  (void);
    
    uint8_t readByte  (uint8_t address);
    bool writeByte  (uint8_t address, uint8_t data);
};

#define BMP180_ADDR 0x77 // 7-bit address
#define CHIP_ID 0x55

#define BMP180_REG_CONTROL_ADDRESS 0xF4
#define BMP180_REG_RESULT_ADDRESS 0xF6
#define BMP180_REG_CHIP_ID_ADDRESS 0xD0
#define BMP180_REG_CALIBRATING_REGISTERS_BEGIN_ADDRESS 0xAA

#define DATA_READY_MASK 1<<5

#define PRESSURE_SIZE 3
#define TEMP_SIZE 2

#define CALIBRATING_REGISTERS_SIZE  11*2

#define	BMP180_COMMAND_TEMPERATURE 0x2E
#define	BMP180_COMMAND_PRESSURE0 0x34

#endif
