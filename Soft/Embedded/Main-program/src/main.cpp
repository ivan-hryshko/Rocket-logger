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

#define WRITE_PERIOD_MS 8

void setup()
{
    Serial.begin(115200); // Open serial communications and wait for port to open:
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
}

typedef struct
{
    uint8_t buff[512];
} data_to_write_struct;

data_to_write_struct data;

void loop()
{
    static SD_log SD;
    uint32_t curr_time = millis();
    strcpy(reinterpret_cast<char *>(&data.buff[0]), "this is data from struct\n");
    strcpy(reinterpret_cast<char *>(&data.buff[100]), String(curr_time).c_str());
    SD.write(&data, sizeof(data));
    Serial.println("Write done");
    while (true)
    {
    }
    static uint32_t start_time = curr_time;
    static uint32_t prev_measure_time = curr_time;
    static uint32_t writes_num = 0;
    if ((curr_time - WRITE_PERIOD_MS) >= prev_measure_time)
    {
        prev_measure_time += WRITE_PERIOD_MS;

        writes_num++;
        if (writes_num >= 1000)
        {
            Serial.print("Total time: ");
            Serial.println(millis() - start_time);
            // if (f_close(_fil) != FR_OK)
            // {
            //     card_error_handler("file close error");
            // }
            // card_error_handler("file write done");
        }
    }
}