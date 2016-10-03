#include <OneWire.h> 
#include <SoftwareSerial.h>

int DS18S20_Pin = 3; //DS18S20 Signal pin on digital 2
int D2_Pin = 3;
SoftwareSerial lcd(5,6);

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2


void setup(void) {
  Serial.begin(9600);
  lcd.begin(9600);
}

void loop(void) {
  float temperature = getTemp();
  float far = toFar(temperature);
  clearDisplay();
  setLCDCursor(0);
  lcd.print(far);
  lcd.print(" F");
  setLCDCursor(16);
  lcd.print(temperature);
  lcd.print(" C");
  delay(100); //just here to slow down the output so it is easier to read
  
}

void setLCDCursor(byte cursor_position)
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x80);  // send the set cursor command
  lcd.write(cursor_position);  // send the cursor position
}

void clearDisplay()
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x01);  // send the clear screen command
}

float toFar(float tep) {
  float far = ((9 * tep) / 5) + 32;
  return far; 
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
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

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
