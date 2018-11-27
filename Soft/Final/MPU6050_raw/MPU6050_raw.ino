#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include "MPU6050.h"
#include "BMP180.h"
#include "I2Cdev.h"
#include "crc16.c"
#include "cameraControl.c"

/* TODO:
  Apogee detect
  Camera control
  NRF24 launch* 
 */

#define MEAS_DEBUG  0
#define FUSE_FIRE_TIMEOUT  1000
#define COUNTDOWN_TIMEOUT 120000
#define LOG_TIMEOUT_AFTER_PARACHUTE 60000
#define NO_PARACHUTE_TIME 8000
#define FORCED_PARA_LAUNCH_TIMEOUT  15000

#define mpuInterruptPin         2
#define sdCardCSpin             8
#define batteryMeasurePin       A7
#define batteryMeasureEnablePin A1
#define camRecPin               3
#define camOnPin                4
#define ledPin                  5
#define paraPin                 7
#define paraChekPin             6
#define enginePin               A2
#define engineChekPin           A0
#define buzzerPin               A3
#define nrfCSpin                9
#define nrfCEpin                10

const char fileName [] = "data.bin";

//#define error(s) sd.errorHalt(F(s))

void logToCard (void);
void fusesInit (void);
void sdCardInit (void);
void battMeasureInit (void);
void i2cInit (void);
void mpu6050Init  (void);
void bmp180Init  (void);
void mpuDataReadyIRQ  (void);
void writeBufferToFile  (void);
void writeBufferToSDcard  (void);
void writeDataFromMpu (uint8_t * toSend);
void writeDataFromBmp (uint8_t * toSend);
void writeBatteryLevel  (uint8_t * toSend);
void writeTimeFromStart (uint8_t * toSend);
void writeSystemStates (uint8_t * toSend);
void writeMeasureNum (uint8_t * toSend);
void writeCRC16 (uint8_t * toSend);
void writeMeasureBorders (uint8_t * toSend);
void addToBuffer (uint8_t * toSend);
void batteryLevelUpdate   (void);
void writeMeasuredDataToCard (void);
bool parachuteCheck (void);
bool engineCheck (void);
void parachuteFireFuse (void);
void engineFireFuse (void);
void engineDisableFuse (void);
void parachuteDisableFuse (void);
void beeperInit (void);
void beeperOn (void);
void beeperOff  (void);
void ledInit  (void);
void ledOn (void);
void ledOff  (void);
void cameraInit (void);
void cameraOn (void);
void cameraOff  (void);
void cameraRecStart (void);
void cameraRecStop  (void);
void radioSignalUpdate  (void);
bool apogeeDetect (void);
void searchingMode (void);

MPU6050 mpu6050;
BMP180 bmp180;

SdFat sd;
SdFile file;

uint8_t * dataBuffer;
uint32_t measureNum;
uint16_t pressureRaw = 0;
int32_t pressureSmooth = 0;
uint32_t countodwnStartTime = 0;
uint32_t engineLaunchTime = 0;
uint32_t paraLaunchTime = 0;
bool newPressureData = false;

void setup()
{
    
    Serial.begin(250000);
    Serial.println();
    Serial.println("Power up");
    Serial.print("GPIO init...");
    beeperInit();
    beeperOn();
    cameraInit();
    cameraOn();
    fusesInit();
    ledInit();
    Serial.println("OK");
    sdCardInit();
    battMeasureInit();
    
    i2cInit();
    bmp180Init();
    mpu6050Init();
    beeperOff();
    delay(500);
    
    Serial.print("engine ");
    if (engineCheck())
    {
      Serial.println("OK");
      beeperOn();
      delay(200);
      beeperOff();
      delay(700);
    }
    else
    {
      Serial.println("FAILURE!!!");
    }
    
    Serial.print("parachute ");
    if (parachuteCheck())
    {
      Serial.println("OK");
      for (uint8_t k = 2; k > 0; k--)
      {
        beeperOn();
        delay(100);
        beeperOff();
        delay(50);
      }
    }
    else
    {
      Serial.println("FAILURE!!!");
    }
    noInterrupts();
    measureNum = 0;
    interrupts();
    countodwnStartTime = millis();
    if (engineCheck())
    {
      cameraRecStart();
      Serial.println("Logging sterted!");
      logToCard();  //start main logging loop
    }
    else
    {
      Serial.println("No engine fuse, launch aborted");
    }
    searchingMode();
}

void searchingMode  (void)
{
  cameraRecStop();
  delay(3000);
  cameraOff();
  while (true)
  {
    beeperOn();
    ledOn();
    delay(100);
    ledOff();
    delay(400);
    beeperOff();
    delay(500);
  }
}

void cameraInit (void)
{
  pinMode(camOnPin, INPUT);
  digitalWrite(camRecPin, HIGH);
  pinMode(camRecPin, INPUT);
}

void cameraOn (void)
{
  pinMode(camOnPin, OUTPUT);
  digitalWrite(camOnPin, HIGH);
  delay(2000);
  pinMode(camOnPin, INPUT);
}

void cameraOff (void)
{
  cameraOn();
}

void cameraRecStart (void)
{
  pinMode(camRecPin, OUTPUT);
  digitalWrite(camRecPin, HIGH);
  delay(400);
  pinMode(camRecPin, INPUT);
}

void cameraRecStop (void)
{
  cameraRecStart();
}

void beeperInit (void)
{
  pinMode(buzzerPin, OUTPUT);
  beeperOff();
}

void beeperOn (void)
{
  digitalWrite(buzzerPin, HIGH);
}

void beeperOff (void)
{
  digitalWrite(buzzerPin, LOW);
}

void ledInit  (void)
{
  pinMode(ledPin, OUTPUT);
  ledOff();
}
void ledOn (void)
{
  digitalWrite(ledPin, HIGH);
}

void ledOff  (void)
{
  digitalWrite(ledPin, LOW);
}

void fusesInit (void)
{
  pinMode(paraChekPin, INPUT);
  pinMode(engineChekPin, INPUT);
  
  pinMode(paraPin, INPUT);
  pinMode(enginePin, INPUT);
}

bool parachuteCheck (void)
{
  return (digitalRead(paraChekPin));
}

bool engineCheck (void)
{
  return (digitalRead(engineChekPin));
}

void parachuteFireFuse (void)
{
  pinMode(paraPin, OUTPUT);
  digitalWrite(paraPin, HIGH);
}

void engineFireFuse (void)
{
  pinMode(enginePin, OUTPUT);
  digitalWrite(enginePin, HIGH);
}

void parachuteDisableFuse (void)
{
  digitalWrite(paraPin, LOW);
  pinMode(paraPin, INPUT);
}

void engineDisableFuse (void)
{
  digitalWrite(enginePin, LOW);
  pinMode(enginePin, INPUT);
}

void battMeasureInit (void)
{
  pinMode(batteryMeasureEnablePin, OUTPUT);
  digitalWrite(batteryMeasureEnablePin, LOW);
  pinMode(batteryMeasurePin, INPUT);
  analogReference(INTERNAL);
  batteryLevelUpdate ();
}

void successCheck (bool operationSuccess)
{
  if (operationSuccess)
  {
    Serial.println("OK");
  }
  else
  {
    Serial.println("ERROR!!!");
    while(true)
    {}
  }
}

void sdCardInit (void)
{
  uint32_t bgnBlock, endBlock;
  uint32_t freeSpaceOnCard;
  
  Serial.print("SD card init...");
  successCheck(sd.begin(sdCardCSpin, SPI_FULL_SPEED));
  
  sd.remove(fileName);    //delete in final
  
  Serial.print("Free space: ");
  freeSpaceOnCard = sd.freeClusterCount() * sd.blocksPerCluster() * 512;
  Serial.print(freeSpaceOnCard);
  Serial.println(" bytes");
  Serial.print("Free clusters: ");
  //freeSpaceOnCard = 16777216;
  Serial.println(sd.freeClusterCount());
  Serial.print("Creating file...");
  successCheck (file.createContiguous(sd.vwd(), fileName, freeSpaceOnCard));
  
  //Serial.println(sd.freeClusterCount());
  Serial.print("File check...");
  successCheck (file.contiguousRange(&bgnBlock, &endBlock));
  dataBuffer = (uint8_t*)sd.vol()->cacheClear();

  Serial.print("Write test...");
  successCheck (sd.card()->writeStart(bgnBlock, freeSpaceOnCard/512));

  memset(dataBuffer, '-', 512);
  char startMessage [] = "start! ";
  memcpy(dataBuffer, startMessage, sizeof(startMessage) - 1);
  writeBufferToSDcard();
}

void i2cInit (void)
{
  Fastwire::setup(400, true);
}

void mpu6050Init (void)
{
  Serial.print("MPU init...");
  mpu6050.initialize();
  mpu6050.setDLPFMode(1);           //set low-pass filter for gyro and acc 188Hz (maximum)
  mpu6050.setRate(1);               //set 500Hz for acelerometer
  mpu6050.setInterruptMode(1);      //set interrupt active low
  mpu6050.setInterruptDrive(MPU6050_INTDRV_PUSHPULL);   //set interrupt output to push-pull
  mpu6050.setInterruptLatch(MPU6050_INTLATCH_50USPULSE);      //set interrupt until clear
  mpu6050.setInterruptLatchClear(true);             //clear interrupt register at any read operation
  mpu6050.setIntMotionEnabled(false);               //disable motion interrupt
  mpu6050.setIntZeroMotionEnabled(false);           //disable zero gravity interrupt
  mpu6050.setIntDataReadyEnabled(true);             //ENABLE data ready interrupt
  mpu6050.setIntI2CMasterEnabled(false);            //disable i2c interrupt
  mpu6050.setIntFIFOBufferOverflowEnabled(false);   //disable FIFO overflow interrupt
  mpu6050.setFIFOEnabled(false);                    //disable FIFO, totaly
  mpu6050.setI2CMasterModeEnabled(false);           //disable i2c master
  mpu6050.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  mpu6050.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  successCheck(mpu6050.testConnection());
  delay(5);
  pinMode(mpuInterruptPin, INPUT); //mpu interrupt init
  attachInterrupt(0, mpuDataReadyIRQ, FALLING);
}

void bmp180Init (void)
{
  Serial.print("BMP init...");
  successCheck(bmp180.begin());
  
  memset(dataBuffer, ' ', 512);
  Fastwire::readBuf(BMP180_ADDR << 1, BMP180_REG_CALIBRATING_REGISTERS_BEGIN_ADDRESS, &dataBuffer[0], CALIBRATING_REGISTERS_SIZE);
  writeBufferToSDcard();
  bmp180.startPressure();
  while (false)
  {
    Serial.println(bmp180.readByte(BMP180_REG_CONTROL_ADDRESS));
  }
}

const uint8_t accSize = 2;
const uint8_t mpuTempSize = 2;
const uint8_t gyroSize = 2;
const uint8_t mpuDataSize = 3*accSize + 3*gyroSize + mpuTempSize;
const uint8_t bmpTempSize = 2;
const uint8_t pressureSize = 2; 
const uint8_t bmpDataSize = bmpTempSize + pressureSize;
const uint8_t millisSize = 4;
const uint8_t batteryChargeSize = 1;
const uint8_t systemStatesSize = 1;
const uint8_t measureNumSize = 4;
const uint8_t CRC16Size      = 2;
const uint8_t CRC16FirstElement = 1;
const uint8_t CRC16LastElement  = 28;
const uint8_t CRC16InputLength  = CRC16LastElement - CRC16FirstElement + 1;
const uint8_t endSize = 1;
const uint8_t singleMeasureSize = 32;

#define FIRST_BORDER_POS  0
#define MPU_START_POS     1
#define BMP_START_POS    15
#define TIME_POS         19
#define BATT_POS         23
#define SYSTEM_STATE_POS 24
#define MEASURE_NUM_POS  25
#define CRC16_POS        29
#define LAST_BORDER_POS  31

uint8_t toSend [singleMeasureSize];

uint8_t bufferOffset = 0;
bool bufferFull = false;

const uint8_t queueSize = 1;
uint8_t queueBuffer [singleMeasureSize * queueSize];
bool queueBufferUsed = false;

volatile bool newMpuDataReady = false;

void mpuDataReadyIRQ (void)
{  
  newMpuDataReady = true;
  measureNum++;
}

uint32_t lastBatteryReadTime = 0;
const uint16_t batteryReadPeriod = 500;

void logToCard (void)
{
  bool logEnable = true;
  bool rocketLaunched = false;
  bool engineFuseEnabled = false;
  bool apogeeCrossed = false;
  bool parachuteFired = false;
  bool paraFuseEnabled = false;
  uint32_t parachuteLaunchTime;
  
  while (logEnable)
  {
    if (bufferFull)
  {
    beeperOn();
    writeMeasuredDataToCard ();
    beeperOff();
  }
  else if (newMpuDataReady)
  {
    writeDataFromMpu    (toSend);
    writeDataFromBmp    (toSend);
    writeTimeFromStart  (toSend);
    writeBatteryLevel   (toSend);
    writeSystemStates   (toSend);
    writeMeasureNum     (toSend);
    writeCRC16          (toSend);
    writeMeasureBorders (toSend);
    addToBuffer         (toSend);
  }
  else    //free time
  {
    if (!rocketLaunched && (millis() - countodwnStartTime) > COUNTDOWN_TIMEOUT)
    {
      engineFireFuse();
      engineFuseEnabled = true;
      rocketLaunched = true;
      engineLaunchTime = millis();
      Serial.println("Engine started");
    }
    else if (engineFuseEnabled && (millis() - engineLaunchTime) > FUSE_FIRE_TIMEOUT)
    {
      engineDisableFuse();
      engineFuseEnabled = false;
      Serial.println("Engine fuse disabled");
    }
    else if (rocketLaunched && ((millis() - engineLaunchTime) > NO_PARACHUTE_TIME) && newPressureData && !apogeeCrossed)
    {
      apogeeCrossed = apogeeDetect();
      newPressureData = false;
      if (apogeeCrossed)
      {
        parachuteFireFuse();
        paraFuseEnabled = true;
        parachuteFired = true;
        parachuteLaunchTime = millis();
        Serial.println("Parachute auto fired!!");
      }
    }
    else if (parachuteFired)
    {
      if(paraFuseEnabled && (millis() - parachuteLaunchTime) > FUSE_FIRE_TIMEOUT)
      {
        parachuteDisableFuse();
        paraFuseEnabled = false;
        Serial.println("Para fuse disabled");
      }
      else if ( (millis() - parachuteLaunchTime) > LOG_TIMEOUT_AFTER_PARACHUTE)
      {
        logEnable = false;
        Serial.println ("logging disabled" );
      }
    }

    if (rocketLaunched && !parachuteFired && (millis() - engineLaunchTime) > FORCED_PARA_LAUNCH_TIMEOUT)
    {
      Serial.println("Para forced fired!");
      parachuteFireFuse();
      paraFuseEnabled = true;
      parachuteFired = true;
      parachuteLaunchTime = millis();
    }
    
    if ((millis() - lastBatteryReadTime) > batteryReadPeriod)
    {
      lastBatteryReadTime = millis();
      batteryLevelUpdate ();
    }
  }
  }
}

void loop() {
  
}

uint32_t pressureLastNotIncrasingTime;
uint8_t pressureThresholdsNum = 0;
#define PRESS_DIFF_GAIN 4

bool apogeeDetect (void)
{
  if (pressureSmooth == 0)
  {
    pressureSmooth = pressureRaw;
    pressureLastNotIncrasingTime = millis();
    return (false);
  }
  if ((pressureRaw - pressureSmooth) > PRESS_DIFF_GAIN)
  {
    pressureThresholdsNum++;
    //Serial.println(pressureRaw - pressureSmooth);
    if(pressureThresholdsNum > 5)
    {
      return (true);
    }
  }
  else
  {
    pressureThresholdsNum = 0;
    pressureLastNotIncrasingTime = millis();
  }
  pressureSmooth = pressureSmooth/2 + pressureRaw/2 ;
  return (false);
}

uint32_t measureNumToWrite = 0;

void writeMeasureNum (uint8_t * toSend)
{
  memcpy(&toSend [MEASURE_NUM_POS], &measureNumToWrite, measureNumSize);
}

void writeMeasureBorders (uint8_t * toSend)
{
  toSend[FIRST_BORDER_POS] = '[';
  toSend[ LAST_BORDER_POS] = ']';
}

void writeDataFromMpu (uint8_t * toSend)
{
  noInterrupts();
  Fastwire::readBuf(MPU6050_DEFAULT_ADDRESS << 1, MPU6050_RA_ACCEL_XOUT_H, &toSend[1], mpuDataSize);  
  measureNumToWrite = measureNum;
  newMpuDataReady = false;
  interrupts();
  if (MEAS_DEBUG)
  {
    Serial.print('m');
  }
}

#define MEASURE_TEMP_INTERVAL 10
uint8_t measureBmpNum = 0;

void writeDataFromBmp (uint8_t * toSend)
{
  memset(&toSend[BMP_START_POS], 0x00, bmpDataSize);
  if(bmp180.checkDataReady())
  {
    measureBmpNum++;
    if (measureBmpNum >= MEASURE_TEMP_INTERVAL)
    { 
      uint8_t * pTemp = bmp180.getTemperature();

      if(MEAS_DEBUG)
      {
        Serial.print('t');
      }
      memcpy(&toSend[15], pTemp, bmpTempSize);
      measureBmpNum = 0;
      bmp180.startPressure();
    }
    else
    {
      uint8_t * pPressure = bmp180.getPressure();

      pressureRaw = *pPressure;
      pressureRaw = pressureRaw*256 + *(pPressure + 1);
      newPressureData = true;

      if (MEAS_DEBUG)
      {
        Serial.print('p');
      }
      memcpy(&toSend[17], pPressure, pressureSize);
      if (measureBmpNum == (MEASURE_TEMP_INTERVAL - 1))
      {
        bmp180.startTemperature();
      }
      else
      {
        bmp180.startPressure();
      }
    }
  }
}

void writeTimeFromStart (uint8_t * toSend)
{
  uint32_t milisec;
  milisec = millis();
  memcpy(&toSend [TIME_POS], &milisec, millisSize);
}

void writeCRC16 (uint8_t * toSend)
{
  uint16_t CRC16;
  
  CRC16 = getCRC16(&toSend[1], CRC16InputLength);
  memcpy(&toSend [CRC16_POS], &CRC16, CRC16Size);
}

bool radioSignal;
void radioSignalUpdate  (void)
{
  
}

uint8_t batteryLevel;
void batteryLevelUpdate (void)
{
  batteryLevel = analogRead(batteryMeasurePin) >> 2;
  if (MEAS_DEBUG)
  {
    Serial.print("b=");
    Serial.print(batteryLevel);
  }
}

void writeBatteryLevel (uint8_t * toSend)
{
  toSend [BATT_POS] = batteryLevel;
}

void writeSystemStates (uint8_t * toSend)
{
  uint8_t systemStates;
  
  systemStates = digitalRead(enginePin)<<0 + digitalRead(paraPin)<<1 + digitalRead(engineChekPin)<<2 + digitalRead(paraChekPin)<<3 + digitalRead(ledPin)<<4 + radioSignal<<5;
  memcpy(&toSend [SYSTEM_STATE_POS], &systemStates, systemStatesSize);
}

void addToBuffer (uint8_t * toSend)
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
  writeBufferToSDcard();
  bufferFull = false;
  if (MEAS_DEBUG)
  {
    Serial.println ('w');
  }
}

void writeBufferToSDcard  (void)
{
  if (!sd.card()->writeData(dataBuffer))
  {
    Serial.println("writeData failed");
  }
}
