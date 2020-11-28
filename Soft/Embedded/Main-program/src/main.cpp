/*
  SD card datalogger
 This example shows how to log data from three analog sensors
 to an SD card using the SD library.
 The circuit:
 * analog sensors on analog ins A0, A1, and A2
 * SD card
 */

#include "Arduino.h"
#include "sd_log.h"
#include "MPU9250.h"
#include "BMP280_DEV.h"
#include "ArduinoLog.h"
#include <string.h>
#include <Wire.h>

#define MAIN_LOG_LEVEL LOG_LEVEL_NOTICE

#define MAIN_PERIOD 1000

BMP280_DEV bmp; // use I2C interface
MPU9250 MPU(Wire, 0x68);
Logging main_log;

void setup()
{
    // Wire.setSDA(PB11);
    // Wire.setSCL(PB10);
    Serial.begin(921600); // Open serial communications and wait for port to open:
    main_log.begin(MAIN_LOG_LEVEL, &Serial);
    main_log.notice("Waiting for monitor open\n");
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
    main_log.notice("Logger start\n");
    Wire.begin((uint8_t)PB11, (uint8_t)PB10);
    int status = MPU.begin();
    if (status < 0)
    {
        Serial.print("MPU init fail: ");
        Serial.println(status);
        while (1)
        {
        }
    }
    else
    {
        MPU.setAccelRange(MPU9250::ACCEL_RANGE_16G);
        MPU.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);
        MPU.setSrd(0);
        // MPU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
        // MPU.enableDataReadyInterrupt();
        main_log.notice("MPU inited\n");
    }

    if (bmp.begin(NORMAL_MODE, BMP280_I2C_ADDR) == 0)
    {
        main_log.fatal(F("Could not find a valid BMP280 sensor, check wiring!"));
        while (1)
        {
        }
    }
    else
    {
        bmp.setPresOversampling(OVERSAMPLING_X1);
        bmp.setTempOversampling(OVERSAMPLING_X1);
        bmp.setIIRFilter(IIR_FILTER_OFF);
        bmp.setTimeStandby(TIME_STANDBY_05MS);
        Wire.setClock(400000);

        //reset sensor to default parameters.
        // bmp.resetToDefaults();

        bmp.startNormalConversion();

        main_log.notice("BMP inited\n");
    }

    analogReadResolution(8);

    pinMode(PB4, OUTPUT);
    digitalWrite(PB4, HIGH);
    delay(50);
    digitalWrite(PB4, LOW);

    main_log.notice("init done\n");
}

typedef struct __attribute__((packed))
{
    uint8_t num;          // 1 byte
    uint32_t micros;      // 4 bytes
    uint8_t batt_voltage; // 1 byte
    struct
    {
        bool bmp_data:1;
        bool mpu_data:1;
        bool engine_fuse_check:1;
        bool engine_fuse_fired:1;
        bool parachute_fuse_check:1;
        bool parachute_fuse_fired:1;
    } __attribute__((packed)) flags;        // 1 byte

    struct
    {
        uint16_t temp;  // 2 bytes
        uint16_t press; // 2 bytes
    } __attribute__((packed)) bmp;
    struct
    {
        int16_t temp; // 2 bytes

        int16_t acc_x; // 2 bytes
        int16_t acc_y; // 2 bytes
        int16_t acc_z; // 2 bytes

        int16_t gyr_x; // 2 bytes
        int16_t gyr_y; // 2 bytes
        int16_t gyr_z; // 2 bytes

        int16_t mag_x; // 2 bytes
        int16_t mag_y; // 2 bytes
        int16_t mag_z; // 2 bytes
    } __attribute__((packed)) mpu;

    uint8_t reserved[1];
} measured_values_t;

static_assert (sizeof(measured_values_t) == 32, "measured_values_t should be 32bytes");

void loop()
{
    static SD_log SD;
    static uint8_t measure_num = 0;

    uint32_t start_time = micros();
    static uint32_t last_measure_time = start_time - MAIN_PERIOD;

    if (start_time - last_measure_time >= MAIN_PERIOD)
    {
        measured_values_t current_meas;
        last_measure_time += MAIN_PERIOD;

        current_meas.num = measure_num++;
        current_meas.micros = start_time;

        if (MPU.readSensor() > 0)
        {
            current_meas.mpu.temp = MPU._tcounts;

            current_meas.mpu.acc_x = MPU._axcounts;
            current_meas.mpu.acc_y = MPU._aycounts;
            current_meas.mpu.acc_z = MPU._azcounts;

            current_meas.mpu.gyr_x = MPU._gxcounts;
            current_meas.mpu.gyr_y = MPU._gycounts;
            current_meas.mpu.gyr_z = MPU._gzcounts;

            current_meas.mpu.mag_x = MPU._hxcounts;
            current_meas.mpu.mag_y = MPU._hycounts;
            current_meas.mpu.mag_z = MPU._hzcounts;

            current_meas.flags.mpu_data = true;
        }
        else
        {
            current_meas.flags.mpu_data = false;
        }

        uint32_t mpu_time = micros();

        float bmp_temp = 0, bmp_pressure = 0;
        if (bmp.getTempPres(bmp_temp, bmp_pressure))
        {
            // Serial.print(bmp_temp);
            // Serial.print(" ");
            // Serial.println(bmp_pressure);
            current_meas.bmp.temp = bmp_temp*10;
            current_meas.bmp.press = bmp_pressure*10;
            current_meas.flags.bmp_data = true;
        }
        else
        {
            current_meas.flags.bmp_data = false;
        }

        uint32_t bmp_time = micros();
        // for (uint8_t j = 0; j < sizeof(current_meas); j++)
        // {
        //     Serial.print(((uint8_t *)(&current_meas))[j], HEX);
        //     Serial.print(' ');
        // }
        // Serial.println();
        SD.write(&current_meas, sizeof(current_meas));
        uint32_t sd_write_time = micros();
        if ((sd_write_time - start_time) > 1500)
        {
            main_log.warning("Long main cycle mpu:%dus\tpres:%dus\tsd:%dus\ttotal:%dus\n", (mpu_time - start_time), (bmp_time - mpu_time), (sd_write_time - bmp_time), (sd_write_time - start_time));
        }
    }
}