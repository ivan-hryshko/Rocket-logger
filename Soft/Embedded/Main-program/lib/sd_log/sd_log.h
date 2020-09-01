#include "ff_gen_drv.h"
#include <queue> // std::queue
#include <array>
#include "ArduinoLog.h"

#ifndef SD_LOG_LEVEL
#define SD_LOG_LEVEL LOG_LEVEL_WARNING
#endif // SD_LOG_LEVEL

#define FILENAME "test.bin"

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

    void tx_done_cb(void);
    void card_error_handler(const char *msg);
    bool write_to_file(uint8_t buffer[512]);
};