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
#include <string.h>

void setup()
{
    Serial.begin(921600); // Open serial communications and wait for port to open:
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
}

typedef struct __attribute__((packed))
{
    uint8_t num;          // 1 byte
    uint32_t micros;      // 4 bytes
    uint8_t batt_voltage; // 1 byte
    uint8_t flags;        // 1 byte

    uint16_t bmp_temp;  // 2 bytes
    uint16_t bmp_press; // 2 bytes

    uint16_t mpu_temp; // 2 bytes

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
    for (uint16_t i = 0; i < 1000; i++)
    {
        // Serial.print("new data: ");
        // Serial.println(i);
        uint32_t curr_time = micros();
        memcpy(&data.buff[0], &i, sizeof(i));
        strcpy(reinterpret_cast<char *>(&data.buff[sizeof(i)]), "this is payload");
        strcpy(reinterpret_cast<char *>(&data.buff[20]), String(curr_time).c_str());
        // for (uint8_t j = 0; j < sizeof(data); j++)
        // {
        //     Serial.print (((uint8_t*)(&data))[j], HEX);
        //     Serial.print (' ');
        // }
        // Serial.println();
        SD.write(&data, sizeof(data));
    }
    Serial.println("Write to file done");
    while (true)
    {
    }
}