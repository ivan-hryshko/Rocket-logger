#include "sd_log.h"
#include "bsp_sd.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "Arduino.h"
#include <functional>

// this is just for handling c-style pointers to class member function
template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)>
{
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }
    static std::function<Ret(Params...)> func;
};

// Initialize the static member.
template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

SD_log::SD_log()
{

    Serial.println("Initializing SD card:");
    // auto c_callback = [=](){tx_done_cb();};
    Callback<void(void)>::func = std::bind(&SD_log::tx_done_cb, this);                            // bind member function with objext
    void (*c_style_cb)(void) = static_cast<decltype(c_style_cb)>(Callback<void(void)>::callback); // convert binded callback to C-style cb
    BSP_SD_set_tx_callback(c_style_cb);
    if (FATFS_LinkDriver(&Custom_SD_Driver, SDPath) != 0)
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
    uint8_t buff[512];

    strncpy((char *)buff, "this is new header", sizeof(buff));
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

    delay(100);
}

void SD_log::tx_done_cb(void)
{
    Serial.println("we are here");
}

void SD_log::write(void *data, size_t size)
{
    // if ()
    Serial.println(String("write ") + String(size) + " bytes");
    UINT written_len = 0;
    FRESULT ret = f_write(_fil, data, size, &written_len);
    if (ret == FR_OK)
    {
        Serial.print("buff written: ");
        Serial.println(written_len);
    }
    else
    {
        Serial.print("error file write: ");
        Serial.println(ret);
    }
}

void SD_log::card_error_handler(const char *msg)
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
