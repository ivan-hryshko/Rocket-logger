# FILE_NAME_INPUT = "input_data.bin"
FILE_NAME_INPUT = "TEST.BIN"
BUFF_SETUP=[
#   size   +   *   sign   name
    [1,    0,  1,  False, "num"],
    [4,    0,  1,  False, "micros"],
    [1,    0,  1,  False, "batt_voltage"],
    [1,    0,  1,  False, "flags"],
    [2,    0,  1,  False, "bmp_temp"],
    [2,    0,  1,  False, "bmp_press"],
    [2,    0,  1,  True,  "mpu_temp"],
    [2,    0,  1,  True,  "mpu_acc_x"],
    [2,    0,  1,  True,  "mpu_acc_y"],
    [2,    0,  1,  True,  "mpu_acc_z"],
    [2,    0,  1,  True,  "mpu_gyr_x"],
    [2,    0,  1,  True,  "mpu_gyr_y"],
    [2,    0,  1,  True,  "mpu_gyr_z"],
    [2,    0,  1,  True,  "mpu_mag_x"],
    [2,    0,  1,  True,  "mpu_mag_y"],
    [2,    0,  1,  True,  "mpu_mag_z"],
    [1,    0,  1,  True,  "reserved"]
]
MAX_BUFF_IN_PACK = len(BUFF_SETUP)

buff_counter = 0
buff = [None] * MAX_BUFF_IN_PACK
def print_buff_console ():
    for buf in buff:
        int_data = buf
        print("{0:>12}".format(int_data),end = ' ')
    print ("")


with open(FILE_NAME_INPUT,"rb")as bin_file:
    while (byte := bin_file.read(BUFF_SETUP[buff_counter][0])):
        int_data = int.from_bytes(byte, byteorder = 'little', signed=BUFF_SETUP[buff_counter][3])
        buff[buff_counter] = int ((int_data + BUFF_SETUP[buff_counter][1]) * BUFF_SETUP[buff_counter][2])
        buff_counter +=1
        if buff_counter >= MAX_BUFF_IN_PACK:
            buff_counter = 0
            print_buff_console()