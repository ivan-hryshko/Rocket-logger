
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

float vectorLength;

void setup() {

    Wire.begin();
    Serial.begin(250000);
    accelgyro.initialize();

    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
}


int32_t quadAX, quadAY, quadAZ;

void loop() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
/*
  quadAX = pow(ax, 2);
  quadAY = pow(ay, 2);
  quadAZ = pow(az, 2);
  vectorLength = sqrt(quadAX + quadAY + quadAZ);
  
  Serial.println(vectorLength); 
      */ 
 /*
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.println(az);
  */
  
  
       Serial.print(gx); Serial.print("\t");
       Serial.print(gy); Serial.print("\t");
      Serial.println(gz);

   
}
