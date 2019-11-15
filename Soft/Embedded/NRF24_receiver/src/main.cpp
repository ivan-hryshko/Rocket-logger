#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
Serial.println("Start");
}

void loop()
{
  Serial.println("Acc:" + String(random(0, 23000)/1000.0));
  Serial.println("Hgh:" + String(random(0, 1000)));
  Serial.println("Par:" + String(random(0, 2)));
  Serial.println("Ang:" + String(random(0, 18000)/100.0));
  Serial.println("---------------------");

  delay(1000);
}