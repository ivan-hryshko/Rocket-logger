#include <SD.h>

File file;
void setup() {
 Serial.begin(9600);
 Serial.println ("Start");
 SD.begin(10);
 file = SD.open("WR_TEST2.TXT", O_CREAT | O_WRITE);

 while(millis() < 1000);  // delay so mills() is four digits

 for (uint8_t i = 0; i < 100; i++) {
   file.println(millis());
 }
 // not needed in this example since close() will call flush.
 file.flush();  

 file.close();
 Serial.println ("End");
}
void loop() {}
