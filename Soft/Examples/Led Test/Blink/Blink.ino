#define enginePin A2
#define ledPin 5
#define parashutPin 7
#define buzzerPin A3

void setup() {
 pinMode(enginePin, OUTPUT);
 pinMode(parashutPin, OUTPUT);
 pinMode(ledPin, OUTPUT);
 pinMode(buzzerPin, OUTPUT);
}



void loop() {
  digitalWrite(enginePin, HIGH);  
  digitalWrite(parashutPin, HIGH);
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  delay(500);                     
  digitalWrite(enginePin, LOW);   
  digitalWrite(parashutPin, LOW);   
  digitalWrite(ledPin, LOW); 
  digitalWrite(buzzerPin, LOW); 
  delay(500);     
}
