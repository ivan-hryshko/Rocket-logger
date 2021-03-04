/*
  SD card datalogger
 This example shows how to log data from three analog sensors
 to an SD card using the SD library.
 The circuit:
 * analog sensors on analog ins A0, A1, and A2
 * SD card
 */

#include <string.h>
#include "Wire.h"
#include "Arduino.h"
#include "sd_log.h"
#include "MPU9250.h"
#include "BMP280_DEV.h"
#include "ArduinoLog.h"
#include "pins.h"
#include "I2CScanner.h"

#define MAIN_LOG_LEVEL LOG_LEVEL_NOTICE

#define MAIN_PERIOD 1000

static BMP280_DEV bmp; // use I2C interface
static MPU9250 mpu(Wire, 0x68);
static Logging main_log;
static SD_log record_file;
static I2CScanner scanner;

void beep (void)
{
    digitalWrite(BOOZER_PIN, HIGH);
    delay(50);
    digitalWrite(BOOZER_PIN, LOW);
}

enum class sensor_type {
    BMP,
    MPU
};

void i2c_device_init_fail (String string, enum sensor_type sensor)
{
    const uint32_t speeds [] = {50, 100, 200, 400};
    uint8_t n = 0;
    bool init_done = false;
    while (init_done == false)
    {
        main_log.fatal(string.c_str());
        main_log.notice("Set speed %d: %dk\n", n, speeds[n]);
        Wire.setClock(speeds[n] * 1000);
        scanner.Scan();

        main_log.notice("Trying init again...\n");
        switch (sensor)
        {
        case sensor_type::BMP:
        {
            uint8_t bmp_init_status = bmp.begin(NORMAL_MODE, BMP280_I2C_ADDR);
            if (bmp_init_status != 1)
            {
                main_log.warning("BMP init error\n");
                init_done = false;
            }
            else
            {
                init_done = true;
            }
            break;
        }

        case sensor_type::MPU:
            int mpu_init_status = mpu.begin();
            if (mpu_init_status != 0)
            {
                main_log.warning("MPU init error: %d\n", mpu_init_status);
                init_done = false;
            }
            else
            {
                init_done = true;
            }
            break;
        }

        digitalWrite(BRIGHT_LED_PIN, HIGH);
        delay(100);
        digitalWrite(BRIGHT_LED_PIN, LOW);
        n = (n + 1) % (sizeof(speeds)/sizeof(speeds[0]));

        delay(500);
    }

    main_log.notice("Senspr %d init success\n", sensor);
}

void setup()
{
    // Wire.setSDA(PB11);
    // Wire.setSCL(PB10);
    Serial.begin(921600); // Open serial communications and wait for port to open:
    pinMode(BRIGHT_LED_PIN, OUTPUT);
    pinMode(BOOZER_PIN, OUTPUT);
    main_log.begin(MAIN_LOG_LEVEL, &Serial);
    // main_log.notice("Waiting for monitor open\n");
    // while (!Serial)
    // {
    //     ; // wait for serial port to connect.
    // }

    beep();

    main_log.notice("Logger start\n");
    if (record_file.init())
    {
        main_log.notice("Record file created\n");
    }
    else
    {
        while (1)
        {
            main_log.fatal("record file create fail\n");
            delay(1000);
        }
    }

    beep();

    Wire.begin((uint8_t)PB11, (uint8_t)PB10);

    scanner.Init();

    if (bmp.begin(NORMAL_MODE, BMP280_I2C_ADDR) == 0)
    {
        i2c_device_init_fail("BMP init fail\n", sensor_type::BMP);
    }

    bmp.setPresOversampling(OVERSAMPLING_X1);
    bmp.setTempOversampling(OVERSAMPLING_X1);
    bmp.setIIRFilter(IIR_FILTER_OFF);
    bmp.setTimeStandby(TIME_STANDBY_05MS);
    Wire.setClock(400000);

    //reset sensor to default parameters.
    // bmp.resetToDefaults();

    bmp.startNormalConversion();

    main_log.notice("BMP inited\n");

    delay(200);

    beep();

    int status = mpu.begin();
    if (status < 0)
    {
        i2c_device_init_fail(String ("MPU init fail: ") + status + '\n', sensor_type::MPU);
    }

    mpu.setAccelRange(MPU9250::ACCEL_RANGE_16G);
    mpu.setGyroRange(MPU9250::GYRO_RANGE_2000DPS);
    mpu.setSrd(0);
    // mpu.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
    // mpu.enableDataReadyInterrupt();
    main_log.notice("MPU inited\n");


    // analogReadResolution(8);

    digitalWrite(BRIGHT_LED_PIN, HIGH);
    delay(100);
    digitalWrite(BRIGHT_LED_PIN, LOW);

    beep();

    main_log.notice("init done\n");
}

typedef struct __attribute__((packed))
{
    uint8_t num;          // 1 byte
    uint32_t micros;      // 4 bytes
    uint8_t batt_voltage; // 1 byte

    struct
    {
        bool bmp_data:1;
        bool mpu_data:1;
        bool engine_fuse_check:1;
        bool engine_fuse_fired:1;
        bool parachute_fuse_check:1;
        bool parachute_fuse_fired:1;
    } __attribute__((packed)) flags;        // 1 byte

    struct
    {
        uint16_t temp;  // 2 bytes
        uint16_t press; // 2 bytes
    } __attribute__((packed)) bmp;
    struct
    {
        int16_t temp; // 2 bytes

        int16_t acc_x; // 2 bytes
        int16_t acc_y; // 2 bytes
        int16_t acc_z; // 2 bytes

        int16_t gyr_x; // 2 bytes
        int16_t gyr_y; // 2 bytes
        int16_t gyr_z; // 2 bytes

        int16_t mag_x; // 2 bytes
        int16_t mag_y; // 2 bytes
        int16_t mag_z; // 2 bytes
    } __attribute__((packed)) mpu;

    uint8_t reserved[1];
} measured_values_t;

static_assert (sizeof(measured_values_t) == 32, "measured_values_t should be 32bytes");

void loop()
{
    static uint8_t measure_num = 0;

    uint32_t start_time = micros();
    static uint32_t boot_timestamp = start_time;
    uint32_t time_to_write = start_time - boot_timestamp;
    static uint32_t last_measure_time = start_time - MAIN_PERIOD;

    if (start_time - last_measure_time >= MAIN_PERIOD)
    {
        static measured_values_t current_meas;
        last_measure_time += MAIN_PERIOD;

        current_meas.num = measure_num++;
        current_meas.micros = time_to_write;

        if (mpu.readSensor() > 0)
        {
            current_meas.mpu.temp = mpu._tcounts;

            current_meas.mpu.acc_x = mpu._axcounts;
            current_meas.mpu.acc_y = mpu._aycounts;
            current_meas.mpu.acc_z = mpu._azcounts;

            current_meas.mpu.gyr_x = mpu._gxcounts;
            current_meas.mpu.gyr_y = mpu._gycounts;
            current_meas.mpu.gyr_z = mpu._gzcounts;

            current_meas.mpu.mag_x = mpu._hxcounts;
            current_meas.mpu.mag_y = mpu._hycounts;
            current_meas.mpu.mag_z = mpu._hzcounts;

            current_meas.flags.mpu_data = true;
        }
        else
        {
            current_meas.flags.mpu_data = false;
        }

        uint32_t mpu_time = micros();

        float bmp_temp = 0, bmp_pressure = 0;
        if (bmp.getTempPres(bmp_temp, bmp_pressure))
        {
            // Serial.print(bmp_temp);
            // Serial.print(" ");
            // Serial.println(bmp_pressure);
            current_meas.bmp.temp = bmp_temp*10U;
            current_meas.bmp.press = bmp_pressure*10U;
            current_meas.flags.bmp_data = true;
        }
        else
        {
            current_meas.flags.bmp_data = false;
        }

        uint32_t bmp_time = micros();

        record_file.write(&current_meas, sizeof(current_meas));

        uint32_t sd_write_time = micros();

        if ((sd_write_time - start_time) > 1500)
        {
            main_log.warning("Long main cycle mpu:%dus\tpres:%dus\tsd:%dus\ttotal:%dus\n", (mpu_time - start_time), (bmp_time - mpu_time), (sd_write_time - bmp_time), (sd_write_time - start_time));
        }
    }
}

// void SystemClock_Config(void)
// {
//   RCC_OscInitTypeDef RCC_OscInitStruct = {};
//   RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
//   RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

//   /* Initializes the CPU, AHB and APB busses clocks */
//   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//   RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
//   RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
//   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
//   RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
//   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
//     Error_Handler();
//   }
//   /* Initializes the CPU, AHB and APB busses clocks */
//   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
//                                 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
//   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
//   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

//   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
//     Error_Handler();
//   }
//   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_USB;
//   PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
//   PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
//   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
//     Error_Handler();
//   }
// }