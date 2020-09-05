/*
  SD card datalogger
 This example shows how to log data from three analog sensors
 to an SD card using the SD library.
 The circuit:
 * analog sensors on analog ins A0, A1, and A2
 * SD card
 */

#include "Arduino.h"
#include "sd_log.h"
#include "MPU9250.h"
#include "BMx280I2C.h"
#include <string.h>
#include <Wire.h>

#define MAIN_PERIOD 1000

BMx280I2C bmp(0x77); // use I2C interface
MPU9250 MPU(Wire, 0x68);

void setup()
{
    Serial.begin(921600); // Open serial communications and wait for port to open:
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
    Serial.println("Logger start");
    Wire.setSDA(PB11);
    Wire.setSCL(PB10);
    Wire.begin();
    Wire.setClock(1000000);
    // int status = MPU.begin();
    // if (status < 0)
    // {
    //     Serial.print("MPU init fail: ");
    //     Serial.println(status);
    //     while (1)
    //     {
    //     }
    // }
    // else
    // {
    //     Serial.println("MPU inited");
    // }
    if (bmp.begin() == 0)
    {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
        while (1)
        {
        }
    }
    else
    {
        if (bmp.isBME280())
        {
            Serial.println("sensor is a BME280");
        }
        else
        {
            Serial.println("sensor is a BMP280");
        }

        //reset sensor to default parameters.
        bmp.resetToDefaults();

        //by default sensing is disabled and must be enabled by setting a non-zero
        //oversampling setting.
        //set an oversampling setting for pressure and temperature measurements.
        bmp.writeOversamplingPressure(BMx280MI::OSRS_P_x01);
        bmp.writeOversamplingTemperature(BMx280MI::OSRS_T_x01);
        bmp.writePowerMode(BMx280MI::BMx280_MODE_NORMAL);
        bmp.writeFilterSetting(BMx280MI::FILTER_OFF);
        bmp.writeStandbyTime(BMx280MI::T_SB_1);
        if (!bmp.measure())
        {
            Serial.println("could not start measurement, is a measurement already running?");
            return;
        }
        // bmp.setPresOversampling(OVERSAMPLING_SKIP);    // Set the pressure oversampling to X4
        // bmp.setTempOversampling(OVERSAMPLING_SKIP); // Set the temperature oversampling to X1
        // bmp.setIIRFilter(IIR_FILTER_OFF);
        // bmp.setTimeStandby(TIME_STANDBY_05MS);
        Serial.println("BMP inited");
    }
    // bmp.startNormalConversion();
    // MPU.setAccelRange(MPU9250::ACCEL_RANGE_16G);
    // MPU.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);
    // MPU.setSrd(0);
    // MPU.enableDataReadyInterrupt();

    analogReadResolution(8);

    pinMode(PB4, OUTPUT);
    digitalWrite(PB4, HIGH);
    delay(50);
    digitalWrite(PB4, LOW);

    // MPU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);

    // MPU.setSrd(19);
    Serial.println("init done");
}

typedef struct __attribute__((packed))
{
    uint8_t num;          // 1 byte
    uint32_t micros;      // 4 bytes
    uint8_t batt_voltage; // 1 byte
    uint8_t flags;        // 1 byte

    uint16_t bmp_temp;  // 2 bytes
    uint16_t bmp_press; // 2 bytes

    int16_t mpu_temp; // 2 bytes

    int16_t mpu_acc_x; // 2 bytes
    int16_t mpu_acc_y; // 2 bytes
    int16_t mpu_acc_z; // 2 bytes

    int16_t mpu_gyr_x; // 2 bytes
    int16_t mpu_gyr_y; // 2 bytes
    int16_t mpu_gyr_z; // 2 bytes

    int16_t mpu_mag_x; // 2 bytes
    int16_t mpu_mag_y; // 2 bytes
    int16_t mpu_mag_z; // 2 bytes
    uint8_t reserved[1];

} measured_values_t;

typedef struct __attribute__((packed))
{
    uint8_t buff[32];
} data_to_write_struct;

data_to_write_struct data;

void loop()
{

    while (true)
    {
        // float bmp_temp = 0, bmp_pressure = 0;
        if (bmp.hasValue())
        {
            Serial.print(bmp.getPressure());
            Serial.print(" ");
            Serial.println(bmp.getTemperature());
        }
        else
        {
            // Serial.println("BMP not ready");
        }
        //delay(100);
    }

    static SD_log SD;
    static uint8_t measure_num = 0;

    uint32_t curr_time = micros();
    static uint32_t last_measure_time = curr_time - MAIN_PERIOD;

    if (curr_time - last_measure_time >= MAIN_PERIOD)
    {
        last_measure_time += MAIN_PERIOD;

        // bmp.startForcedConversion();
        measured_values_t current_meas =
            {
                .num = measure_num++,
                .micros = curr_time,
                .batt_voltage = 0, //static_cast<uint8_t>(analogRead(PC0)),
                .flags = 0,

                .bmp_temp = 0,
                .bmp_press = 0,

                .mpu_temp = 0,
                .mpu_acc_x = 0,
                .mpu_acc_y = 0,
                .mpu_acc_z = 0,
                .mpu_gyr_x = 0,
                .mpu_gyr_y = 0,
                .mpu_gyr_z = 0,
                .mpu_mag_x = 0,
                .mpu_mag_y = 0,
                .mpu_mag_z = 0,

                .reserved = {0},

            };
        // float bmp_temp = 0, bmp_pressure = 0;
        // static uint32_t bmp_meas_time = micros() - 10000;
        // if ((curr_time - bmp_meas_time >= 10000) && bmp.getTempPres(bmp_temp, bmp_pressure))
        // {
        //     bmp_meas_time += 10000;
        //     Serial.print(bmp_temp);
        //     Serial.print(" ");
        //     Serial.println(bmp_pressure);
        //     current_meas.bmp_temp = bmp_temp*10;
        //     current_meas.bmp_press = bmp_pressure/10;
        // }
        // else
        // {
        //     // Serial.println("BMP not ready");
        // }
        if (MPU.readSensor() > 0)
        {
            current_meas.mpu_temp = MPU._tcounts;

            current_meas.mpu_acc_x = MPU._axcounts;
            current_meas.mpu_acc_y = MPU._aycounts;
            current_meas.mpu_acc_z = MPU._azcounts;

            current_meas.mpu_gyr_x = MPU._gxcounts;
            current_meas.mpu_gyr_y = MPU._gycounts;
            current_meas.mpu_gyr_z = MPU._gzcounts;

            current_meas.mpu_mag_x = MPU._hxcounts;
            current_meas.mpu_mag_y = MPU._hycounts;
            current_meas.mpu_mag_z = MPU._hzcounts;
        }
        Serial.println(current_meas.mpu_acc_z);
        // for (uint8_t j = 0; j < sizeof(current_meas); j++)
        // {
        //     Serial.print(((uint8_t *)(&current_meas))[j], HEX);
        //     Serial.print(' ');
        // }
        // Serial.println();
        SD.write(&current_meas, sizeof(current_meas));
        // Serial.println(micros() - curr_time);
    }
}