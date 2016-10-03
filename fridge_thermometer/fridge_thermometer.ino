#include <SoftwareSerial.h>
#include <OneWire.h> 

const int freezer_temp = 6;
const int fridge_temp = 7;
const int freezer_light = 4;
const int fridge_light = 5;
const int speaker = 9;
const int mute_button = 8;

boolean mute = false;

const int softwareTx = 3;
SoftwareSerial s7s(4,softwareTx);
char tempString[10];

OneWire freezerDS(freezer_temp);
OneWire fridgeDS(fridge_temp);

void setup() 
{
  pinMode(freezer_light, OUTPUT);
  pinMode(fridge_light,OUTPUT);
  pinMode(mute_button, INPUT);
  
  s7s.begin(9600);
  clearDisplay();
  setDecimals(0b00000100);
  
  digitalWrite(freezer_light,LOW);
  digitalWrite(fridge_light,LOW);
  
}

void loop() {
  
  if(digitalRead(mute_button) == HIGH) {
      mute = true;
  }
  
  int ztemp = getFreezer();
  sprintf(tempString, "%4d", ztemp);
  clearDisplay();
  setDecimals(0b00000100);
  s7s.print(tempString);
  digitalWrite(fridge_light,LOW);
  digitalWrite(freezer_light,HIGH);
  if(ztemp > 33 && !mute) {
   tone(speaker, 494, 2000); 
  }
  delay(2000);
  noTone(speaker);
  
  int gtemp = getFridge();
  sprintf(tempString, "%4d", gtemp);
  clearDisplay();
  setDecimals(0b00000100);
  s7s.print(tempString);
  digitalWrite(fridge_light,HIGH);
  digitalWrite(freezer_light,LOW);
  
  if(gtemp > 40 && !mute) {
    tone(speaker,494, 2000);
  }
  delay(2000);
  noTone(speaker);
}

void setDecimals(byte decimals)
{
  s7s.write(0x77);
  s7s.write(decimals);
}

void clearDisplay()
{
  s7s.write(0x76);  // Clear display command
}

// get the temp off the DS18S20
int getFreezer(){
  //returns the temperature from one DS18S20 in DEG Celsius
  int return_int;

  byte data[12];
  byte addr[8];

  if ( !freezerDS.search(addr)) {
      //no more sensors on chain, reset search
      freezerDS.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  freezerDS.reset();
  freezerDS.select(addr);
  freezerDS.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = freezerDS.reset();
  freezerDS.select(addr);    
  freezerDS.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = freezerDS.read();
  }
  
  freezerDS.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  TemperatureSum = (TemperatureSum * 1.8) + 32;
  return_int = (int)(TemperatureSum * 10);
  return return_int;
}

// get the temp off the DS18S20
int getFridge(){
  //returns the temperature from one DS18S20 in DEG Celsius
  int return_int;

  byte data[12];
  byte addr[8];

  if ( !fridgeDS.search(addr)) {
      //no more sensors on chain, reset search
      fridgeDS.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  fridgeDS.reset();
  fridgeDS.select(addr);
  fridgeDS.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = fridgeDS.reset();
  fridgeDS.select(addr);    
  fridgeDS.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = fridgeDS.read();
  }
  
  fridgeDS.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  TemperatureSum = (TemperatureSum * 1.8) + 32;
  return_int = (int)(TemperatureSum * 10);
  return return_int;
}
