#include <SoftwareSerial.h>
#include <OneWire.h> 

SoftwareSerial Serial7Segment(3,2);

int pentPin = 0;
int pentValue = 0;

int change_timeout = 15; // HOW MANY LOOPS TO DISPLAY CHNAGED TEMP
int change_timer = 0;
int last_pent = 0;
int last_temp = 0;
int range=5; // HOW MANY DEGREES TO GO UP OR DOWN FROM SETTING 10 = 1

char tempString[10];

//Temperature chip i/o
int DS18S20_Pin = 4; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2
int temp_correction = -7;

void setup() {
  Serial7Segment.begin(9600);
  Serial1.begin(9600);
  resetLCD();
  setBrightness(128);
  clearDisplay(); 
}

void loop() {
  
  pentValue = (int)((analogRead(pentPin) / 34) + 50) * 10;

  float temperature = getTemp();
  float farh = toFarh(temperature);
  int tempi = (int)(farh * 10) + (temp_correction * 10);
 
  
  if(tempi < -100) { // FIX ERRORS
     tempi = last_temp; 
  }

  // if dial was changed show the dial
  if(pentValue != last_pent) {
    last_pent = pentValue; 
    change_timer = 0;
  }
  
  int display_int = 0;
  
  change_timer++;
  if(change_timer < change_timeout) {
    display_int = pentValue;   
    resetLCD();  
  } else {
    change_timer = change_timeout;
    display_int = tempi;
  }
  
  sprintf(tempString, "%4d", display_int);

  Serial7Segment.print(tempString);
  
  if(tempi < pentValue - range) {
    Serial1.print('N');
  } else if(tempi > pentValue + range) {
    Serial1.print('F');
  }
  
  last_temp = tempi;
  delay(100);
}

float getVoltage(int pin)
{
  return (analogRead(pin) * .004882814); 
}

void setDecimals(byte decimals)
{
  Serial7Segment.write(0x77);
  Serial7Segment.write(decimals);
}

void resetLCD()
{
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
  Serial7Segment.write(0x77);  // Special command to turn on specific light with following command
  Serial7Segment.write(0b00000100);  // special command to turn on the decimal point
}

// given a celcius temp return a farhenight
float toFarh(float cel)
{
  return ((cel * 1.8) + 32.0);
}

// get the temp off the DS18S20
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

void clearDisplay()
{
  Serial7Segment.write(0x76);  // Clear display command
}

void setBrightness(byte value)
{
  Serial7Segment.write(0x7A);  // Set brightness command byte
  Serial7Segment.write(value);  // brightness data byte
}
