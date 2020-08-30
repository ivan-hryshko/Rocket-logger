#include "ff_gen_drv.h"

#define FILENAME "test.bin"

class SD_log
{
public:
    SD_log(); // constructor
    void write(void *data, size_t size);

private:
    void tx_done_cb(void);
    FIL ff_struct = {};
    FIL *_fil = &ff_struct;
    char SDPath[4]; // SD card logical drive path
    FATFS SDFatFs;  /* File system object for SD card logical drive */

    void card_error_handler(const char *msg);
};