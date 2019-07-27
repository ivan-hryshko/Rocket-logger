#include <Arduino.h>
#include "pins.h"

void setup()
{
	pinMode(BRIGHT_LED_PIN, OUTPUT);
	if (true)
	return;
}

void loop()
{
	digitalWrite(BRIGHT_LED_PIN, HIGH);
	delay(1000);
	digitalWrite(BRIGHT_LED_PIN, LOW);
	delay(1000);
}