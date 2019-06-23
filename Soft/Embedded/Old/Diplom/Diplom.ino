#include <Wire.h>

//TwoWire WIRE2 (2, I2C_FAST_MODE);
#define Wire WIRE2

#include "I2Cdev.h"
#include "MPU6050.h"

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;

#define ENGINE_START_FUSE_PIN     PB5
#define PARACHUTE_LAUNCH_FUSE_PIN PB7
#define BRIGHT_LED_PIN            PB6

void initDevices (void)
{
  //Serial.begin(500000);
  while (millis() < 7000)
  {
    Serial.println ((7000 - millis()) / 1000);
    delay (1000);
  }

  Serial.println("Initializing I2C devices...");
  Wire.begin();
  Serial.println("wire inited");
  accelgyro.initialize();

  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
}


void waitEngineStart (uint32_t launchTimeMs)
{
  Serial.println("Engine start countdown");
  while (millis() < launchTimeMs)
  {
    Serial.println((launchTimeMs - millis()) / 1000);
    delay(1000);
  }
  pinMode (ENGINE_START_FUSE_PIN, OUTPUT);
  digitalWrite (ENGINE_START_FUSE_PIN, HIGH);
  delay (1000);
  digitalWrite(ENGINE_START_FUSE_PIN, LOW);
}

bool isRocketFliped (void)
{
  int16_t accX, accY, accZ;

  accelgyro.getAcceleration(&accX, &accY, &accZ);

  Serial.println(String(accX) + String('\t') + String(accY));
  if ((accX > 5500) && (accY < -5500))
  {
    Serial.println("Rocked flipped!!!");
    return (true);
  }
  else
  {
    return (false);
  }
}

void launchParachute (void)
{
  Serial.println("Launching parachute");
  pinMode (PARACHUTE_LAUNCH_FUSE_PIN, OUTPUT);
  digitalWrite (PARACHUTE_LAUNCH_FUSE_PIN, HIGH);
  delay (1000);
  digitalWrite(PARACHUTE_LAUNCH_FUSE_PIN, LOW);
}

void blinkLedForewer (void)
{
  Serial.println("Led blinking start");
  pinMode (BRIGHT_LED_PIN, OUTPUT);
  while (true)
  {
    digitalWrite (BRIGHT_LED_PIN, HIGH);
    delay (1000);
    digitalWrite(BRIGHT_LED_PIN, LOW);
    delay (1000);
  }
}

void tripleBlink (void)
{
  pinMode (BRIGHT_LED_PIN, OUTPUT);
  for (uint8_t i = 0; i < 3; i++)
  {
    digitalWrite (BRIGHT_LED_PIN, HIGH);
    delay (100);
    digitalWrite(BRIGHT_LED_PIN, LOW);
    delay (100);
  }
}

void setup()
{
  tripleBlink();
  initDevices();
  waitEngineStart(10000);
  while (isRocketFliped() == false)
  {
    delay(1);
  }
  launchParachute ();
  blinkLedForewer ();
}

void loop()
{

}
