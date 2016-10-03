#include <SoftwareSerial.h>

SoftwareSerial mySerial(3,4); 

unsigned int rpm = 0;
unsigned long last_time;

void setup() {
  mySerial.begin(9600);
  attachInterrupt(0, tick, RISING);
  setBrightness(255);  // High brightness
  clearDisplay();  
}

void loop() {
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", rpm);
  clearDisplay();   
  mySerial.write(tempString);
  delay(400);
}

void tick() {
  unsigned long now = millis();
  unsigned long time = now - last_time;
  last_time = now;
  rpm = (int)(60000 / time);
}

void clearDisplay()
{
  mySerial.write(0x76);  // Clear display command
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightness(byte value)
{
  mySerial.write(0x7A);  // Set brightness command byte
  mySerial.write(value);  // brightness data byte
}

