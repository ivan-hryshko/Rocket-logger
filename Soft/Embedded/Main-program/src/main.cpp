/*
  SD card datalogger
 This example shows how to log data from three analog sensors
 to an SD card using the SD library.
 The circuit:
 * analog sensors on analog ins A0, A1, and A2
 * SD card
 */

// #include <STM32SD.h>
#include "Sd2Card.h"
#include "SdFatFs.h"
#include "Arduino.h"
#include <string.h>

#define WRITE_PERIOD_MS 10
#define FILENAME "test.bin"

// If SD card slot has no detect pin then define it as SD_DETECT_NONE
// to ignore it. One other option is to call 'SD.begin()' without parameter.
#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN SD_DETECT_NONE
#endif

SdFatFs _fatFs;
FIL ff_struct = {};
FIL *_fil = &ff_struct;
void setup()
{
    Sd2Card _card;
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for Leonardo only
    }

    Serial.print("Initializing SD card...");
    // see if the card is present and can be initialized:
    if (_card.init(SD_DETECT_PIN) && _fatFs.init())
    {
        Serial.println("card initialized.");
    }
    else
    {
        Serial.println("card init fail !!!");
        while (true)
        {
        }
    }

    FILINFO fno;
    uint8_t mode = FILE_WRITE;
    if ((mode == FILE_WRITE) && (f_stat(FILENAME, &fno) != FR_OK))
    {
        mode = mode | FA_CREATE_ALWAYS;
    }

    if (f_open(_fil, FILENAME, mode) == FR_OK)
    {
        Serial.println("File opened");
    }
    else
    {
        Serial.println("File open error !!!");
    }

    if (f_expand(_fil, 10 * 1024 * 1024, 1) == FR_OK)
    { /* Check if the file has been expanded */
        Serial.println("Failed to allocate contiguous area.");
    }
    else
    {
        Serial.println("File size reserved");
    }
    f_close(_fil);
    Serial.println("all done");
}

uint8_t buff[512];

void loop()
{
    // static uint32_t prev_measure_time = 0;
    // static uint32_t writes_num = 0;
    // uint32_t curr_time = millis();
    // if ((curr_time - WRITE_PERIOD_MS) >= prev_measure_time)
    // {
    //     prev_measure_time = curr_time;
    //     Serial.print("write ");
    //     Serial.println(writes_num++);
    //     memcpy(buff, &curr_time, sizeof(curr_time));
    //     if (dataFile)
    //     {
    //         Serial.println(dataFile.write(buff, sizeof(buff)));
    //     }
    //     else
    //     {
    //         Serial.println("error opening datalog.txt");
    //     }
    // }
}