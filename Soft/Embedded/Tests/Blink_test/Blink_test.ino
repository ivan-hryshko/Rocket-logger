#define BRIGHT_LED PB6

void setup()
{
  Serial.begin (500000);
  pinMode(BRIGHT_LED, OUTPUT);
  delay(8000);  //this pause neeaded for usb connection
  Serial.println ("Start");
}

void loop()
{
  Serial.print ('.');
  digitalWrite(BRIGHT_LED, HIGH);
  delay(1000);
  digitalWrite(BRIGHT_LED, LOW);
  delay(1000);
}
