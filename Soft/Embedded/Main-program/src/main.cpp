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
#include "Adafruit_BMP280.h"
#include <string.h>
#include <Wire.h>

Adafruit_BMP280 bmp; // use I2C interface
MPU9250 MPU(Wire, 0x68);

void setup()
{
    Serial.begin(921600); // Open serial communications and wait for port to open:
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
    Serial.println("Logger start");
    Wire.setSDA(PB11);
    Wire.setSCL(PB10);
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
        Serial.println("MPU inited");
    }
    if (!bmp.begin())
    {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
        while (1)
        {
        }
    }
    else
    {
        Serial.println("BMP inited");
    }
    MPU.setAccelRange(MPU9250::ACCEL_RANGE_16G);
    MPU.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);
    MPU.enableDataReadyInterrupt();

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,   /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_NONE, /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_NONE, /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    analogReadResolution(8);

    pinMode(PB4, OUTPUT);
    digitalWrite(PB4, HIGH);
    delay (50);
    digitalWrite(PB4, LOW);

    // MPU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);

    // MPU.setSrd(19);
    Serial.println("init done");
}

typedef struct __attribute__((packed))
{
    uint8_t num;          // 1 byte
    uint32_t micros;      // 4 bytes
    uint8_t batt_voltage; // 1 byte
    uint8_t flags;        // 1 byte

    uint16_t bmp_temp;  // 2 bytes
    uint16_t bmp_press; // 2 bytes

    int16_t mpu_temp; // 2 bytes

    int16_t mpu_acc_x; // 2 bytes
    int16_t mpu_acc_y; // 2 bytes
    int16_t mpu_acc_z; // 2 bytes

    int16_t mpu_gyr_x; // 2 bytes
    int16_t mpu_gyr_y; // 2 bytes
    int16_t mpu_gyr_z; // 2 bytes

    int16_t mpu_mag_x; // 2 bytes
    int16_t mpu_mag_y; // 2 bytes
    int16_t mpu_mag_z; // 2 bytes
    uint8_t reserved[1];

} measured_values_t;

typedef struct __attribute__((packed))
{
    uint8_t buff[32];
} data_to_write_struct;

data_to_write_struct data;

void loop()
{
    static SD_log SD;
    static uint8_t measure_num = 0;

    uint32_t curr_time = micros();
    static uint32_t last_measure_time = curr_time - 1000;

    if (curr_time - last_measure_time >= 1000)
    {
        last_measure_time += 1000;
        MPU.readSensor();
        measured_values_t current_meas =
            {
                .num = measure_num++,
                .micros = curr_time,
                .batt_voltage = 0,  //static_cast<uint8_t>(analogRead(PC0)),
                .flags = 0,

                .bmp_temp = 0xADDE,
                .bmp_press = 0xEFBE,

                .mpu_temp = MPU._tcounts,

                .mpu_acc_x = MPU._axcounts,
                .mpu_acc_y = MPU._aycounts,
                .mpu_acc_z = MPU._azcounts,

                .mpu_gyr_x = MPU._gxcounts,
                .mpu_gyr_y = MPU._gycounts,
                .mpu_gyr_z = MPU._gzcounts,

                .mpu_mag_x = MPU._hxcounts,
                .mpu_mag_y = MPU._hycounts,
                .mpu_mag_z = MPU._hzcounts,

                .reserved = {0},

            };
        // Serial.println(current_meas.mpu_acc_x);
        // for (uint8_t j = 0; j < sizeof(current_meas); j++)
        // {
        //     Serial.print(((uint8_t *)(&current_meas))[j], HEX);
        //     Serial.print(' ');
        // }
        // Serial.println();
        SD.write(&current_meas, sizeof(current_meas));
    }
    else
    {
        // Serial.println("skipped");
    }
    // Serial.println("Write to file done");
    // while (true)
    // {
    // }
}