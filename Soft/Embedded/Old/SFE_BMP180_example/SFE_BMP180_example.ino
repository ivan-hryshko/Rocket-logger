#include "SFE_BMP180.h"
#include <Wire.h>

SFE_BMP180 pressure;

//#define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters

void setup()
{
  Serial.begin(115200);
  Serial.println("REBOOT");

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}

void loop()
{
  char status;
  double tempValue,p0,a;
  uint32_t pressureValue;

  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);

    status = pressure.getTemperature(tempValue);
    if (status != 0)
    {
      //Serial.print("temperature: ");
      Serial.println(tempValue);
      //Serial.println(" C, ");
      
      status = pressure.startPressure(3);
      if (status != 0)
      {
             delay(status);

        status = pressure.getPressure(pressureValue);
        if (status != 0)
        {
          //Serial.print("absolute pressure: ");
          //Serial.println(pressureValue);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

  //delay(100);  // Pause for 5 seconds.
}
