#include <Arduino.h>
#include "pins.h"

void setup()
{
	pinMode(PARACHUTE_LAUNCH_FUSE_PIN, OUTPUT);
}

void loop()
{
	digitalWrite(PARACHUTE_LAUNCH_FUSE_PIN, HIGH);
	delay(1000);
	digitalWrite(PARACHUTE_LAUNCH_FUSE_PIN, LOW);
	delay(1000);
}