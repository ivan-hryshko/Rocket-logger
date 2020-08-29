/*
  SD card datalogger
 This example shows how to log data from three analog sensors
 to an SD card using the SD library.
 The circuit:
 * analog sensors on analog ins A0, A1, and A2
 * SD card
 */

#include "bsp_sd.h"
#include "ff_gen_drv.h"
#include "drivers/sd_diskio.h"
#include "Arduino.h"
#include <string.h>

#define WRITE_PERIOD_MS 16
#define FILENAME "test.bin"

void card_error_handler(const char *msg)
{
    Serial.println();
    Serial.print("SD card error: ");
    Serial.println(msg);
    Serial.println("Halted");
    while (true)
    {
        delay(1);
    }
}

uint8_t buff[512];

FIL ff_struct = {};
FIL *_fil = &ff_struct;
char SDPath[4]; // SD card logical drive path
FATFS SDFatFs;  /* File system object for SD card logical drive */

void setup()
{
    Serial.begin(115200); // Open serial communications and wait for port to open:
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }


    Serial.println("Initializing SD card:");
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        card_error_handler("card link error");
    }
    if (f_mount(&SDFatFs, (TCHAR const *)SDPath, 0) != FR_OK)
    {
        card_error_handler("mount error");
    }

    if (f_open(_fil, FILENAME, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        Serial.println("File opened");
    }
    else
    {
        card_error_handler("File open error !!!");
    }
    FRESULT ret = f_expand(_fil, 10UL * 1024UL * 1024UL, 1);
    if (ret == FR_OK)
    { /* Check if the file has been expanded */
        Serial.println("File size reserved");
    }
    else
    {
        card_error_handler(String("Failed to allocate contiguous area: " + String(ret)).c_str());
    }
    UINT written_len = 0;
    strncpy((char *)buff, "this is header", sizeof(buff));
    if (f_write(_fil, buff, sizeof(buff), &written_len) == FR_OK)
    {
        Serial.println("header write done");
    }
    else
    {
        card_error_handler("File header write err");
    }
    ret = f_sync(_fil);
    if (ret == FR_OK)
    {
        Serial.println("file synced, all done");
    }
    else
    {
        card_error_handler(String("file sync error: " + String(ret)).c_str());
    }

    memset(buff, '0', sizeof(buff));
    strcpy((char *)&buff[30], __TIME__);
}

void loop()
{
    uint32_t curr_time = millis();
    static uint32_t prev_measure_time = curr_time;
    static uint32_t writes_num = 0;
    if ((curr_time - WRITE_PERIOD_MS) >= prev_measure_time)
    {
        prev_measure_time = curr_time;
        Serial.print("write ");
        Serial.println(writes_num);
        memcpy(&buff[0], &writes_num, sizeof(writes_num));
        memcpy(&buff[sizeof(writes_num)], &curr_time, sizeof(curr_time));
        UINT written_len = 0;
        delay(0);
        FRESULT ret = f_write(_fil, buff, sizeof(buff), &written_len);
        if (ret == FR_OK)
        {
            Serial.print("buff written: ");
            Serial.println(written_len);
            // static uint32_t last_sync_time = curr_time;
            // if (curr_time - last_sync_time > 1000)
            // {
            //     if (f_sync(_fil) == FR_OK)
            //     {
            //         Serial.println("sync ok");
            //         last_sync_time = curr_time;
            //     }
            //     else
            //     {
            //         Serial.print("sync err: ");
            //         Serial.println(ret);
            //     }
            // }
        }
        else
        {
            Serial.print("error file write: ");
            Serial.println(ret);
        }
        writes_num++;
        if (writes_num >= 5000)
        {
            if (f_close(_fil) != FR_OK)
            {
                card_error_handler("file close error");
            }
            card_error_handler("file write done");
        }
    }
}