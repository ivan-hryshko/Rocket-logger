#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "MPU6050.h"

#define mpuInterruptPin   2
#define sdCardCSpin       8

void sdCardInit (void);
void mpu6050Init (void);
void mpuDataReadyIRQ (void);

MPU6050 accelgyro;
File dataFile;

const uint16_t bufferSize = 512;
uint8_t sdWriteBuffer [bufferSize];


void setup()
{
    TIMSK0 = 0; //disable timer 0 with millis
    Serial.begin(115200);
    Serial.println("Power up");
    sdCardInit();
    Wire.begin(); //for i2c
    mpu6050Init();
    interrupts();
}

void sdCardInit (void)
{
  Serial.print("Initializing SD card...");
  if (!SD.begin(sdCardCSpin)) {
    Serial.println("Card failed, or not present!!!!!!");
    while (true){}
  }
  Serial.println("card initialized.");
  dataFile = SD.open("datalog.csv", O_CREAT | O_WRITE);
}

void mpu6050Init (void)
{
  Serial.println("MPU init");
  accelgyro.initialize();
  Serial.println("MPU configuring");
  accelgyro.setDLPFMode(1);           //set low-pass filter for gyro and acc 188Hz (maximum)
  accelgyro.setRate(0);               //set 1kHz for acelerometer
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


volatile int16_t ax, ay, az;
volatile int16_t gx, gy, gz;
volatile uint16_t counter = 0;
//volatile String toSend;

void mpuDataReadyIRQ (void)
{
  counter++;
  interrupts();
  Serial.print('=');
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  
  
  //toSend = ax;
  /*
  toSend += ';';
  toSend += ay;
  toSend += ';';
  toSend += az;
  toSend += ';';
  toSend += gx;
  toSend += ';';
  toSend += gy;
  toSend += ';';
  toSend += gz;
  */
  
}


void loop() {  
  noInterrupts();
  uint16_t localCounter = counter;
  //Serial.println('1');
  interrupts();
  //String toSend;
  //toSend = ax;
  //Serial.println('2');
  //Serial.print('-');
  dataFile.write(ax);
  Serial.println(localCounter);
  if (localCounter >= 1000)
  {
    
    Serial.print(" lC:");
    Serial.println(localCounter);
    
    noInterrupts();
    counter = 0;
    file.write(sdWriteBuffer, bufferSize);
    dataFile.flush();
    //dataFile.close();
    //dataFile = SD.open("datalog.csv", FILE_WRITE);
    
    interrupts();
    
  }
  
  
  
/*
  vectorLength = sqrt(quadAX + quadAY + quadAZ);
  
  Serial.println(vectorLength); 
      */ 
 /*
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.println(az);
  */
  
  
      //Serial.print(gx); Serial.print("\t");
      //Serial.print(gy); Serial.print("\t");
      //Serial.println(gz);

   
}


