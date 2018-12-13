#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include "MPU6050.h"
#include "I2Cdev.h"

#define mpuInterruptPin   2
const uint8_t sdCardCSpin = 8;
const char fileName [] = "data.bin";

#define error(s) sd.errorHalt(F(s))

void sdCardInit (void);
void mpu6050Init  (void);
void mpuDataReadyIRQ  (void);
void writeBufferToFile  (void);
void writeBufferToSDcard  (void);
void readDataFromMpu (void);
void readDataFromBmp (void);
void readBatteryVoltage (void);
void addToBuffer (void);
void writeMeasuredDataToCard (void);

MPU6050 accelgyro;

SdFat sd;
SdFile file;

//const uint16_t bufferSize = 512;
//uint8_t sdWriteBuffer [bufferSize];

uint8_t * dataBuffer;

void setup()
{
    //TIMSK0 = 0; //disable timer 0 with millis
    Serial.begin(115200);
    Serial.println("Power up");
    sdCardInit();
    mpu6050Init();
    interrupts();
}



void sdCardInit (void)
{
  uint32_t bgnBlock, endBlock;
  uint32_t freeSpaceOnCard;
  
  Serial.println("Initializing SD card");
  
  if (!sd.begin(sdCardCSpin, SPI_FULL_SPEED))
  {
    sd.initErrorHalt();
  }
  Serial.println("card initialized.");
  
  sd.remove(fileName);    //delete in final
  freeSpaceOnCard = sd.freeClusterCount() * sd.blocksPerCluster() * 512;
  freeSpaceOnCard = 16777216;
  Serial.println(sd.freeClusterCount());
  if (!file.createContiguous(sd.vwd(), fileName, freeSpaceOnCard))
  {
    Serial.println("createContiguous failed");
  }
  Serial.println(sd.freeClusterCount());
  if (!file.contiguousRange(&bgnBlock, &endBlock))
  {
    Serial.println("contiguousRange failed");
  }
  dataBuffer = (uint8_t*)sd.vol()->cacheClear();
  
  if (!sd.card()->writeStart(bgnBlock, freeSpaceOnCard/512))
  {
    Serial.println("writeStart failed");
  }
  memset(dataBuffer, ' ', 512);
  char startMessage [] = "start";
  memcpy(dataBuffer, startMessage, sizeof(startMessage) - 1);
  writeBufferToSDcard();
  memset(dataBuffer, '-', 512);
  writeBufferToSDcard();
  /*
  String numberToString = "9999";
  for (int number = 0; number < 255; number++)
  {
    numberToString = String(number, DEC);
    Serial.println(numberToString);
    //numberToString = String(number);
    memcpy(&dataBuffer[0], &number, sizeof(number));
    writeBufferToSDcard();
  }
  */
  
  Serial.println("end");
  //Serial.println (dataBuffer);
  while(false)
  {}
}

void mpu6050Init (void)
{
  //Wire.begin(); //for i2c
  Fastwire::setup(400, false);
  Fastwire::beginTransmission(MPU6050_DEFAULT_ADDRESS);
  Serial.println("MPU init");
  accelgyro.initialize();
  Serial.println("MPU configuring");
  accelgyro.setDLPFMode(1);           //set low-pass filter for gyro and acc 188Hz (maximum)
  accelgyro.setRate(1);               //set 500Hz for acelerometer
  accelgyro.setInterruptMode(1);      //set interrupt active low
  accelgyro.setInterruptDrive(MPU6050_INTDRV_PUSHPULL);   //set interrupt output to push-pull
  accelgyro.setInterruptLatch(MPU6050_INTLATCH_50USPULSE);      //set interrupt until clear
  accelgyro.setInterruptLatchClear(true);             //clear interrupt register at any read operation
  accelgyro.setIntMotionEnabled(false);               //disable motion interrupt
  accelgyro.setIntZeroMotionEnabled(false);           //disable zero gravity interrupt
  accelgyro.setIntDataReadyEnabled(true);             //ENABLE data ready interrupt
  accelgyro.setIntI2CMasterEnabled(false);            //disable i2c interrupt
  accelgyro.setIntFIFOBufferOverflowEnabled(false);   //disable FIFO overflow interrupt
  accelgyro.setFIFOEnabled(false);                    //disable FIFO, totaly
  accelgyro.setI2CMasterModeEnabled(false);           //disable i2c master
  Serial.println("Testing device connections...");
  //delay(5);
  //Serial.println(accelgyro.getDeviceID());
  Serial.println(accelgyro.testConnection() ? "MPU6050 OK!" : "MPU6050 connection failed!!!");
  delay(5);
  noInterrupts();
  pinMode(mpuInterruptPin, INPUT); //mpu interrupt init
  attachInterrupt(0, mpuDataReadyIRQ, FALLING);
}

const uint8_t accSize = 2;
const uint8_t mpuTempSize = 2;
const uint8_t gyroSize = 2;
const uint8_t mpuDataSize = 3*accSize + 3*gyroSize + mpuTempSize;
const uint8_t bmpTempSize = 2;          //TODO
const uint8_t pressureSize = 2; 
const uint8_t bmpDataSize = bmpTempSize + pressureSize;
const uint8_t millisSize = 4;
const uint8_t batteryChargeSize = 1;
const uint8_t fusesStateSize = 1;
const uint8_t radioStateSize = 1;
const uint8_t measureNumSize = 4;
const uint8_t endSize = 3;
const uint8_t singleMeasureSize = 32;

const char endOfMeasureSymbol = ';';

int16_t ax, ay, az;
int16_t gx, gy, gz;
uint32_t measureNum = 0;
uint8_t toSend [singleMeasureSize];

volatile bool newMpuDataReady = false;
volatile bool newTask = false;

//volatile bool readComplete;
//volatile bool lastReadComplete = true;

uint8_t bufferOffset = 0;
bool bufferFull = false;

const uint8_t queueSize = 1;
uint8_t queueBuffer [singleMeasureSize * queueSize];
bool queueBufferUsed = false;

void mpuDataReadyIRQ (void)
{  
  newMpuDataReady = true;
  newTask = true;
  measureNum++;
}

void loop() {
  int8_t errCode = -1;
  if (newTask)
  {
    if (newMpuDataReady)
    {
      readDataFromMpu ();
      if (false)
      {
        Serial.print (errCode);
        Serial.println(" Mpu read error!!!");
        while (true)
        {}
      }
      readDataFromBmp ();
      
      addToBuffer ();
    }
    else if (bufferFull)
    {
      writeMeasuredDataToCard ();
      if (!newMpuDataReady)
      {
        newTask = false;
      }
      //Serial.print ('w');
    }
  }
  else  //free time
  {
    Serial.print ('f');
    readBatteryVoltage ();
  }
}

//volatile uint8_t errCode;

void readDataFromMpu (void)
{
  //I2Cdev::readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, mpuDataSize, &toSend[0]);
  //int8_t errCode;
  uint32_t microsec = micros();
  Fastwire::readBuf(MPU6050_DEFAULT_ADDRESS << 1, MPU6050_RA_ACCEL_XOUT_H, toSend, 14);
  
  microsec = micros() - microsec;
  Serial.println(microsec);
  newMpuDataReady = false;

  toSend[0] = '|';
  
  toSend[14] = 't';
  toSend[15] = 't';
  toSend[16] = 'p';
  toSend[17] = 'p';
  toSend[31] = ']';
  //memset(&toSend[18], ';', 14);
  memcpy(&toSend [25], &measureNum, measureNumSize);
}

void readDataFromBmp (void)
{
  
}

void readBatteryVoltage (void)
{
  
}

void addToBuffer (void)
{
  if (!bufferFull)
  {
    if (queueBufferUsed)
    {
      memcpy(dataBuffer, queueBuffer, bufferOffset * singleMeasureSize);
      queueBufferUsed = false;
    }
    memcpy(&dataBuffer [bufferOffset * singleMeasureSize], toSend, singleMeasureSize);
  }
  else
  {
    if (bufferOffset > queueSize)
    {
      Serial.println("queueBuffer overloaded!!!!");
      while(true)
      {}
    }
    queueBufferUsed = true;
    memcpy(&queueBuffer [bufferOffset * singleMeasureSize], toSend, singleMeasureSize);
  }

  bufferOffset++;
  if (bufferOffset > 15)
  {
    bufferOffset = 0;
    bufferFull = true;
  }
}

void writeMeasuredDataToCard (void)
{
  //noInterrupts();
  writeBufferToSDcard();
  //interrupts();
  bufferFull = false;
}

void writeBufferToSDcard  (void)
{
  if (!sd.card()->writeData(dataBuffer))
  {
    Serial.println("writeData failed");
  }
}
