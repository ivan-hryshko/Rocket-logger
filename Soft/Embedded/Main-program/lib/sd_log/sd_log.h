#include "ff_gen_drv.h"
#include "CircularBuffer.h"
#include <array>
#include "ArduinoLog.h"

#ifndef SD_LOG_LEVEL
#define SD_LOG_LEVEL LOG_LEVEL_ERROR
#endif // SD_LOG_LEVEL

// #define FILENAME "log"  // index and ".bin" file extension will be added
#define FILE_SIZE   (1UL * 1024UL * 1024UL)    // in bytes

#define BUFFER_SIZE 10  // number of elements in output buffer (each 512 bytes)

#define SD_CARD_SECTOR_SIZE     _MIN_SS

template <typename T, size_t N> struct alignas(4) aligned_array : public std::array<T,N> {};

typedef aligned_array<uint8_t, SD_CARD_SECTOR_SIZE> sd_block_t;

const uint8_t file_header alignas(4) [SD_CARD_SECTOR_SIZE] = "This is file header";

class SD_log
{
public:
    SD_log(); // constructor
    bool init(void);
    void write(void *data, size_t size);

private:
    Logging log;
    FIL file_obj;
    bool file_opened;
    char SDPath[4]; // SD card logical drive path
    FATFS SDFatFs;  /* File system object for SD card logical drive */
    CircularBuffer<sd_block_t, BUFFER_SIZE> write_buff;

    uint16_t file_index;    // used in file name (first number)
    uint16_t file_part_num; // used in file name (second number)

    void tx_done_cb(void);

    uint16_t get_next_file_index(void);
    String get_full_file_name (uint16_t file_index, uint16_t file_part_num);

    void card_error_handler(String msg);

    bool crete_new_file(void);
    bool write_to_file(uint8_t buffer[512]);

    static void before_log(Print* output);
};