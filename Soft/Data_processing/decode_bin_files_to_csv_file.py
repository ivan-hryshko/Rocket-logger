from os import listdir
from os.path import isfile, join
import sys
import csv

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
MAX_BUFF_IN_PACK = len(BUFF_SETUP)


buff = [None] * MAX_BUFF_IN_PACK
def print_buff_console ():
    for buf in buff:
        int_data = buf
        print("{0:>9}".format(int_data),end = ' ')
    print()

if __name__ == "__main__":
    FILE_FOLDER_PATH = sys.argv[1]
allFilesList = [f for f in listdir(FILE_FOLDER_PATH) if isfile(join(FILE_FOLDER_PATH, f))]

sortFileList = []
for files in allFilesList:
    pack = [(files.replace(".BIN","").split('_'))[0]] + [files]
    sortFileList.append(pack)
filepack = dict.fromkeys((part[0] for part in sortFileList),[])
for num in sortFileList:
    filepack[num[0]] = filepack[num[0]] + [num[1]]

def read_version (input_data:bytes):
    text = ""
    for byte in input_data:
        if byte == 0:
            break
        text += chr(byte)
    #     print (chr(byte),end ="")
    # print ("")
    return text

def read_file_pack (fileList: list, tag):
    buff_counter = 0
    print("start decode: " + tag)
    with open(FILE_FOLDER_PATH + "/" + tag + ".csv", mode='w', newline='') as csv_file:
        writer = csv.writer(csv_file)
        for fileName in fileList:
            with open(FILE_FOLDER_PATH+"/"+fileName,"rb")as bin_file:
                if ("_0000.BIN" in fileName):
                    print(read_version(bin_file.read(512)))
                # print ([i[4] for i in BUFF_SETUP], end ='')
                writer.writerow([i[4] for i in BUFF_SETUP])
                while (byte := bin_file.read(BUFF_SETUP[buff_counter][0])):
                    int_data = int.from_bytes(byte, byteorder = 'little', signed=BUFF_SETUP[buff_counter][3])
                    buff[buff_counter] = int ((int_data + BUFF_SETUP[buff_counter][1]) * BUFF_SETUP[buff_counter][2])
                    buff_counter +=1
                    if buff_counter >= MAX_BUFF_IN_PACK:
                        buff_counter = 0
                        writer.writerow(buff)
                        # print_buff_console() 

    print("end decode  : " + tag + "\n")
for num in filepack:
    read_file_pack (filepack[num],num)
    