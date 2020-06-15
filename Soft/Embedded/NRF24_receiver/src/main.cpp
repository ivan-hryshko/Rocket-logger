#include <Arduino.h>
#include <ArduinoJson.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
}

void loop()
{
  StaticJsonDocument<200> json;

  json["Acc"] = random(0, 23000)/1000.0;
  json["Hgh"] = random(0, 1000);
  json["Par"] = random(0, 2);
  json["Ang"] = random(0, 18000) / 100.0;

  serializeJson(json, Serial);
  Serial.print('\n');

  delay(1000);
}
