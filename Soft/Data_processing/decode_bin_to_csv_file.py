import csv
import sys

FILE_NAME_OUTPUT = "output_data.csv"
# FILE_NAME_INPUT = "input_data.bin"
FILE_NAME_INPUT = "TEST.BIN"
BUFF_SETUP=[
#   size  +   *   sign   name
    [1,   0,  1,  False, "num"],
    [4,   0,  1,  False, "micros"],
    [1,   0,  1,  False, "batt_voltage"],
    [1,   0,  1,  False, "flags"],
    [2,   0,  1,  False, "bmp_temp"],
    [2,   0,  1,  False, "bmp_press"],
    [2,   0,  1,  True,  "mpu_temp"],
    [2,   0,  1,  True,  "mpu_acc_x"],
    [2,   0,  1,  True,  "mpu_acc_y"],
    [2,   0,  1,  True,  "mpu_acc_z"],
    [2,   0,  1,  True,  "mpu_gyr_x"],
    [2,   0,  1,  True,  "mpu_gyr_y"],
    [2,   0,  1,  True,  "mpu_gyr_z"],
    [2,   0,  1,  True,  "mpu_mag_x"],
    [2,   0,  1,  True,  "mpu_mag_y"],
    [2,   0,  1,  True,  "mpu_mag_z"],
    [1,   0,  1,  True,  "reserved"]
]
if __name__ == "__main__":
    FILE_NAME_INPUT = sys.argv[1]

MAX_BUFF_IN_PACK = len(BUFF_SETUP)

buff_counter = 0
buff = [0] * MAX_BUFF_IN_PACK
def read_version (input_data:bytes):
    text = ""
    for byte in input_data:
        if byte == 0:
            break
        text += chr(byte)
        print (chr(byte),end ="")
    print ("")
    return text


with open(FILE_NAME_OUTPUT, mode='w', newline='') as csv_file:
    writer = csv.writer(csv_file)
    with open(FILE_NAME_INPUT, "rb")as bin_file:
        writer.writerow([read_version(bin_file.read(512))])
        print ([i[4] for i in BUFF_SETUP], end ='')
        writer.writerow([i[4] for i in BUFF_SETUP])
        while (byte := bin_file.read(BUFF_SETUP[buff_counter][0])):
            int_data = int.from_bytes(byte, byteorder='little', signed=BUFF_SETUP[buff_counter][3])
            buff[buff_counter] =  int ((int_data + BUFF_SETUP[buff_counter][1]) * BUFF_SETUP[buff_counter][2])
            buff_counter += 1
            if buff_counter >= MAX_BUFF_IN_PACK:
                buff_counter = 0
                writer.writerow(buff)