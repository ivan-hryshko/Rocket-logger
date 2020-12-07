from data_map import BUFF_SETUP, MAX_BUFF_IN_PACK
# FILE_NAME_INPUT = "input_data.bin"
FILE_NAME_INPUT = "test_logs/000_0000.BIN"
buff_counter = 0
buff = [None] * MAX_BUFF_IN_PACK
def print_buff_console ():
    for buf in buff:
        int_data = buf
        print("{0:>9}".format(int_data),end = ' ')
    print()


with open(FILE_NAME_INPUT,"rb")as bin_file:
    while (byte := bin_file.read(BUFF_SETUP[buff_counter][0])):
        int_data = int.from_bytes(byte, byteorder = 'little', signed=BUFF_SETUP[buff_counter][3])
        buff[buff_counter] = int ((int_data + BUFF_SETUP[buff_counter][1]) * BUFF_SETUP[buff_counter][2])
        buff_counter +=1
        if buff_counter >= MAX_BUFF_IN_PACK:
            buff_counter = 0
            print_buff_console()