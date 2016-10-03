#include <SoftwareSerial.h>

// These are the Arduino pins required to create a software seiral
//  instance. We'll actually only use the TX pin.
const int softwareTx = 2;
const int softwareRx = 3;

int pentPin = 0;
int pentValue = 0;

SoftwareSerial s7s(softwareRx, softwareTx);

char tempString[10];  // Will be used with sprintf to create strings

void setup()
{
  s7s.begin(9600);
  setBrightness(255);  // High brightness
  clearDisplay();  
}

void loop()
{
  pentValue = (int)(((analogRead(pentPin) / 3.4) + 550));
  sprintf(tempString, "%4d", pentValue);
  
  s7s.print(tempString);
  setDecimals(0b00000100);  // Sets digit 3 decimal on
  delay(100);  // This will make the display update at 10Hz.
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplay()
{
  s7s.write(0x76);  // Clear display command
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightness(byte value)
{
  s7s.write(0x7A);  // Set brightness command byte
  s7s.write(value);  // brightness data byte
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimals(byte decimals)
{
  s7s.write(0x77);
  s7s.write(decimals);
}

