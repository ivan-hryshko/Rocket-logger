#include "sd_log.h"
#include "bsp_sd.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "Arduino.h"
#include "ArduinoLog.h"
#include <functional>
#include <algorithm>
#include <iterator>
#include <array>

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
    log.begin(SD_LOG_LEVEL, &Serial);
    log.notice("Initializing SD card:\n");

    Callback<void(void)>::func = std::bind(&SD_log::tx_done_cb, this);                            // bind member function with objext
    void (*c_style_cb)(void) = static_cast<decltype(c_style_cb)>(Callback<void(void)>::callback); // convert binded callback to C-style cb
    BSP_SD_set_tx_callback(c_style_cb);
    log.verbose("Callback set\n");

    if (FATFS_LinkDriver(&Custom_SD_Driver, SDPath) != 0)
    {
        card_error_handler("card link error\n");
    }
    if (f_mount(&SDFatFs, (TCHAR const *)SDPath, 0) != FR_OK)
    {
        card_error_handler("mount error\n");
    }

    if (f_open(_fil, FILENAME, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        log.trace("File opened\n");
    }
    else
    {
        card_error_handler("File open error !!!");
    }
    FRESULT ret = f_expand(_fil, 10UL * 1024UL * 1024UL, 1);
    if (ret == FR_OK)
    { /* Check if the file has been expanded */
        log.trace("File size reserved\n");
    }
    else
    {
        card_error_handler(String("Failed to allocate contiguous area: " + String(ret)).c_str());
    }
    UINT written_len = 0;
    uint8_t buff[512] = {0};

    strcpy((char *)buff, "this is new header");
    strncpy((char *)&buff[509], "end", 3);
    if (f_write(_fil, buff, sizeof(buff), &written_len) == FR_OK)
    {
        log.trace("header write done\n");
    }
    else
    {
        card_error_handler("File header write err");
    }
    ret = f_sync(_fil);
    if (ret == FR_OK)
    {
        log.trace("file synced, all done\n");
    }
    else
    {
        card_error_handler(String("file sync error: " + String(ret)).c_str());
    }

    delay(100);
}

void SD_log::tx_done_cb(void)
{
    log.verbose("tx_done\n");
    if (write_buff.empty() == false)
    {
        log.trace("Sending from buffer (%d)\n", write_buff.size());
        if (write_to_file(write_buff.front().data()))
        {
            write_buff.pop();
        }
    }
}

void SD_log::write(void *data, size_t size)
{
    static block_512_t buffer;
    static uint16_t buffer_pos = 0;

    if (buffer_pos + size > buffer.max_size())
    {
        log.warning("not aligned data - skipped (%d, %d, %d)\n", buffer_pos, size, buffer.max_size());
        return;
    }
    if (write_buff.size() >= BUFFER_SIZE)
    {
        log.warning("Buffer is full, data lost\n");
        return;
    }

    log.trace("write %d bytes to pos %d\n", size, buffer_pos);
    // std::copy_n(reinterpret_cast<uint8_t *>(data), size, std::begin(buffer) + buffer_pos);
    memcpy (&buffer[buffer_pos], data, size);
    
    buffer_pos += size;


    if (buffer_pos == 512)
    {
        buffer_pos = 0;
        noInterrupts();
        if (write_buff.empty())
        {
            interrupts();
            if (write_to_file(buffer.data()))
            {
                log.trace("Write done\n");
            }
            else
            {
                log.error("Write failed - data lost\n");
            }
        }
        else
        {
            write_buff.push(buffer);
            interrupts();
            log.trace("Data added to buffer (%d)\n", write_buff.size());
        }
    }
}

void SD_log::card_error_handler(const char *msg)
{
    log.fatal("\nSD card error: %s\nHalted\n", msg);
    while (true)
    {
        delay(1);
    }
}

bool SD_log::write_to_file(uint8_t buffer[512])
{
    UINT written_len = 0;
    FRESULT ret = f_write(_fil, buffer, 512, &written_len);
    if (ret == FR_OK)
    {
        log.verbose("buff written: %d\n", written_len);
        return (true);
    }
    else
    {
        log.error("error file write: %d\n", ret);
        return (false);
    }
}
