#include "ff_gen_drv.h"
#include <queue> // std::queue
#include <array>
#include "ArduinoLog.h"

#ifndef SD_LOG_LEVEL
#define SD_LOG_LEVEL LOG_LEVEL_NOTICE
#endif // SD_LOG_LEVEL

#define FILENAME "log"  // index and ".bin" file extension will be added

#define BUFFER_SIZE 10  // number of elements in output buffer (each 512 bytes)

typedef std::array<uint8_t, 512> block_512_t;

class SD_log
{
public:
    SD_log(); // constructor
    void write(void *data, size_t size);

private:
    Logging log;
    FIL ff_struct = {};
    FIL *_fil = &ff_struct;
    char SDPath[4]; // SD card logical drive path
    FATFS SDFatFs;  /* File system object for SD card logical drive */
    std::queue<block_512_t> write_buff;
    uint16_t file_index;
    uint16_t file_part_num;

    void tx_done_cb(void);

    uint16_t get_next_file_index(void);
    String get_full_file_name (uint16_t file_index, uint16_t file_part_num);

    void card_error_handler(String msg);

    bool write_to_file(uint8_t buffer[512]);
};