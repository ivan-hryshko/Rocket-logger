#include "SFE_BMP180.h"
#include <Wire.h>

SFE_BMP180 pressure;

double baseline;
double getPressure();

void setup()
{
  Serial.begin(250000);

  if (!pressure.begin()){
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  
  baseline = getPressure();
  
  Serial.print("baseline pressure: ");
  Serial.print(baseline);
  Serial.println(" mb");  
}

void loop()
{
  double altitudeValue,pressureValue;

  pressureValue = getPressure();
  
  Serial.println(pressureValue);
}


double getPressure()
{
  char status;
  double T,P,p0,a;

  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}



