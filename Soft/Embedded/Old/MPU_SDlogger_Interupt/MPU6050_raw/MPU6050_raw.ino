#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include <Wire.h>
#include "MPU6050.h"
#include "I2Cdev.h"

#define mpuInterruptPin   2
const uint8_t sdCardCSpin = 8;

#define error(s) sd.errorHalt(F(s))

void sdCardInit (void);
void mpu6050Init (void);
void mpuDataReadyIRQ (void);
void writeBufferToFile (void);

MPU6050 accelgyro;

SdFat sd;
SdFile file;

//const uint16_t bufferSize = 512;
//uint8_t sdWriteBuffer [bufferSize];


void setup()
{
    //TIMSK0 = 0; //disable timer 0 with millis
    Serial.begin(115200);
    Serial.println("Power up");
    delay(10);
    sdCardInit();
    Wire.begin(); //for i2c
    mpu6050Init();
    interrupts();
}

void sdCardInit (void)
{
  Serial.print("Initializing SD card...");
  //sd.begin(sdCardCSpin, SPI_FULL_SPEED);
  if (!sd.begin(sdCardCSpin, SPI_FULL_SPEED)) {
    sd.initErrorHalt();
  }
  /*
  if (!sd.begin(sdCardCSpin, SPI_FULL_SPEED)) {
    Serial.println("Card failed, or not present!!!!!!");
    while (true){}
  }
  */
  Serial.println("card initialized.");
  if (!file.open("log.bin", O_CREAT | O_TRUNC | O_RDWR)) {
    Serial.println("open failed");
  }
  file.truncate(0);
}

void mpu6050Init (void)
{
  Serial.println("MPU init");
  accelgyro.initialize();
  Serial.println("MPU configuring");
  accelgyro.setDLPFMode(1);           //set low-pass filter for gyro and acc 188Hz (maximum)
  accelgyro.setRate(1);               //set 500Hz for acelerometer
  accelgyro.setInterruptMode(1);      //set interrupt active low
  accelgyro.setInterruptDrive(MPU6050_INTDRV_PUSHPULL);   //set interrupt output to push-pull
  accelgyro.setInterruptLatch(MPU6050_INTLATCH_50USPULSE);      //set interrupt only for 50us
  accelgyro.setInterruptLatchClear(true);             //clear interrupt register at any read operation
  accelgyro.setIntMotionEnabled(false);               //disable motion interrupt
  accelgyro.setIntZeroMotionEnabled(false);           //disable zero gravity interrupt
  accelgyro.setIntDataReadyEnabled(true);             //ENABLE data ready interrupt
  accelgyro.setIntI2CMasterEnabled(false);            //disable i2c interrupt
  accelgyro.setIntFIFOBufferOverflowEnabled(false);   //disable FIFO overflow interrupt
  accelgyro.setFIFOEnabled(false);                    //disable FIFO, totaly
  accelgyro.setI2CMasterModeEnabled(false);           //disable i2c master
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 OK!" : "MPU6050 connection failed!!!");
  delay(5);
  noInterrupts();
  pinMode(mpuInterruptPin, INPUT); //mpu interrupt init
  attachInterrupt(0, mpuDataReadyIRQ, FALLING);
  
}

const uint8_t pressureSize = 2; 
const uint8_t bmpTempSize = 2;          //TODO
const uint8_t bmpDataSize = bmpTempSize + pressureSize;
const uint8_t mpuTempSize = 2;          //TODO
const uint8_t gyroSize = 2;
const uint8_t accSize = 2;
const uint8_t mpuDataSize = 3*accSize + 3*gyroSize + mpuTempSize;
const uint8_t endSize = 1;
const uint8_t bufferSize = mpuDataSize + bmpDataSize + endSize;

const char endOfMeasureSymbol = ';';

volatile int16_t ax, ay, az;
volatile int16_t gx, gy, gz;
volatile uint16_t counter = 0;
volatile uint8_t toSend [bufferSize];
volatile bool newMpuDataInBuf = false;

void mpuDataReadyIRQ (void)
{  
  counter++;
  interrupts();
  //Serial.print('=');
  I2Cdev::readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, 14, toSend);
  //accelgyro.getMotionBuffer(&toSend[0]);
  
  toSend[14] = 't';
  toSend[15] = 't';
  toSend[16] = 'p';
  toSend[17] = 'p';
  toSend[18] = ';';
  newMpuDataInBuf = true;  
}


void loop() {  
  //noInterrupts();
  //uint16_t localCounter = counter;
  //Serial.println('1');
  //interrupts();
  if  (newMpuDataInBuf)
  {
    noInterrupts();
    file.write(&toSend, sizeof(toSend));
    interrupts();
    file.sync();
  }
  /*
  if (localCounter >= 30)
  {
    
    //Serial.print(" lC:");
    //Serial.println(counter);
    
    //noInterrupts();
    //counter = 0;
    //interrupts();
    //file.sync();
    //file.close();
  }  
  */
}
