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