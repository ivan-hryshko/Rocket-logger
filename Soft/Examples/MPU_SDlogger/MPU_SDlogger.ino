/*
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */

#include <SPI.h>
#include <SD.h>
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

const int chipSelect = 8;

File dataFile;

void setup() {
  Serial.begin(115200);
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (true){}
  }
  Serial.println("card initialized.");
  dataFile = SD.open("datalog.csv", O_CREAT | O_WRITE);

  Wire.begin();
  accelgyro.initialize();
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
}

void loop()
{
  uint16_t k = 1000;
  String toSend;
  while(k > 0)
  {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    toSend = ax;
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
    
    dataFile.println(toSend);
    k--;
  }
  
    dataFile.close();
    Serial.println("Happy end!");
    while (true){}
}









