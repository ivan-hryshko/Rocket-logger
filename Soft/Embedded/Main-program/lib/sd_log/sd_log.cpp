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
#include <climits>

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
    file_opened = false;
    log.setPrefix(&SD_log::before_log);
}

bool SD_log::init(void)
{
    log.begin(SD_LOG_LEVEL, &Serial);
    log.trace("Initializing SD card\n");

    Callback<void(void)>::func = std::bind(&SD_log::tx_done_cb, this);                            // bind member function with objext
    void (*c_style_cb)(void) = static_cast<decltype(c_style_cb)>(Callback<void(void)>::callback); // convert binded callback to C-style cb
    BSP_SD_set_tx_callback(c_style_cb);
    log.verbose("Callback set\n");

    if (FATFS_LinkDriver(&Custom_SD_Driver, SDPath) != 0)
    {
        card_error_handler("card link error");
        return (false);
    }

    if (f_mount(&SDFatFs, (TCHAR const *)SDPath, 1) != FR_OK)
    {
        card_error_handler("mount error (is SD card installed?)");
        return (false);
    }

    file_index = get_next_file_index();
    if (file_index == UINT16_MAX)
    {
        card_error_handler("all files indices are used");
        return (false);
    }
    file_part_num = 0;

    if (crete_new_file() == false)
    {
        log.error("Can't create first log file");
        return (false);
    }

    uint8_t buff[SD_CARD_SECTOR_SIZE] = {0};
    strcpy(reinterpret_cast<char*>(buff), FILE_HEADER);
    write_to_file(buff);
    // delay(200);

    log.trace("SD card inited\n");

    return (true);
}


uint16_t SD_log::get_next_file_index(void)
{
    unsigned int last_index;

    for (last_index = 0; last_index < UINT32_MAX; last_index++)
    {
        String file_to_check = get_full_file_name(last_index, 0);
        if (f_stat(file_to_check.c_str(), NULL) != FR_OK)   // file not found
        {
            break;
        }
        else
        {
            log.verbose("file %s exists", file_to_check.c_str());
        }
        
    }

    return (last_index);
}

String SD_log::get_full_file_name (uint16_t file_index, uint16_t file_part_num)
{
    String full_file_name = String(FILENAME) + '_' + String(file_index) + '_' + String (file_part_num) + ".bin";
    return (full_file_name);
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

void SD_log::before_log(Print* output)
{
    output->print("sd_log ");
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


    if (buffer_pos == SD_CARD_SECTOR_SIZE)
    {
        String branch_type;
        uint32_t start_time = micros();
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
            branch_type = "direct";
        }
        else
        {
            write_buff.push(buffer);
            interrupts();
            log.trace("Data added to buffer (%d)\n", write_buff.size());
            branch_type = "buffer";
        }
        int32_t finish_time = micros();
        if ((finish_time - start_time) > 500)
        {
            log.warning("Long %s write (%dus)\n", branch_type.c_str(), (finish_time - start_time));
        }
    }
}

void SD_log::card_error_handler(String msg)
{
    log.fatal("\nSD card error: %s\nHalted\n", msg.c_str());
    while (true)
    {
        delay(1);
    }
}

bool SD_log::crete_new_file(void)
{
    FRESULT status;

    if (file_opened)
    {
        status = f_close(&file_obj);
        if (status == FR_OK)
        {
            file_part_num++;    // next file will be created with incremented number (part)
            file_opened = false;
        }
        else
        {
            log.fatal("Can't close file\n");
            return (false);
        }
    }

    String full_file_name = get_full_file_name(file_index, file_part_num);
    status = f_open(&file_obj, full_file_name.c_str(), FA_WRITE | FA_CREATE_NEW);
    if (status == FR_OK)
    {
        log.notice("File %s created\n", full_file_name.c_str());
        file_opened = true;
    }
    else
    {
        card_error_handler("Can't create file " + full_file_name + " err:" + status);
        return(false);
    }

    status = f_expand(&file_obj, FILE_SIZE, 1);
    if (status == FR_OK)
    { /* Check if the file has been expanded */
        log.trace("File size reserved\n");
    }
    else
    {
        card_error_handler("Failed to allocate contiguous area: " + status);
        return(false);
    }

    status = f_sync(&file_obj); // sync needed to apply file expansion to flash

    if (status == FR_OK)
    {
        log.trace("file synced, all done\n");
    }
    else
    {
        card_error_handler("file sync error: " + status);
        return (false);
    }

    return (true);
}

bool SD_log::write_to_file(uint8_t buffer[SD_CARD_SECTOR_SIZE])
{
    FRESULT status;
    FSIZE_t file_size = f_size(&file_obj);
    FSIZE_t file_used = f_tell(&file_obj);

    log.verbose("size:%d, pos:%d\t%s%%\n", file_size, file_used, String((100.0f * file_used)/file_size, 1).c_str());
    if (file_used + SD_CARD_SECTOR_SIZE > file_size)
    {
        if (crete_new_file() == false)
        {
            card_error_handler("Can't create new file");
        }
    }

    UINT written_len = 0;
    status = f_write(&file_obj, buffer, SD_CARD_SECTOR_SIZE, &written_len);
    if (status == FR_OK)
    {
        log.trace("buff written: %d\n", written_len);
        return (true);
    }
    else
    {
        log.error("error file write: %d\n", status);
        return (false);
    }
}
