#include <SoftwareSerial.h>
#include <OneWire.h> 

SoftwareSerial Serial7Segment(8,7);

int pentPin = 0;  // A0
int pentValue = 0;

int change_timeout = 15; // HOW MANY LOOPS TO DISPLAY CHNAGED TEMP
int change_timer = 0;
int last_pent = 0;

int relay_pin = 3;
int heat_on = 0;
int nob_led_pin = 4;

//Temperature chip i/o
int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2
int temp_correction = -7;


void setup() {
  pinMode(relay_pin, OUTPUT);
  pinMode(nob_led_pin, OUTPUT);
  Serial7Segment.begin(9600);
  Serial.begin(9600);
  setBrightness(15);
  resetLCD();
}

void loop() {
  
  int temp_off_point = 0;
  int temp_on_point = 0;
  
  float temperature = getTemp();
  float farh = toFarh(temperature);
  
  int tempi = (int)(farh * 10) + (temp_correction * 10);
 
  pentValue = (((int)(analogRead(pentPin) / 20.48) + 30) * 10);
  temp_on_point = pentValue - 20;
  temp_off_point = pentValue + 20;
  
  // PRINT DATA TO THE SERIAL LOGS
  Serial.print("Pent: ");
  Serial.println(pentValue);
  Serial.print("Temp: ");
  Serial.println(tempi);
  Serial.print("ON: ");
  Serial.println(temp_on_point);
  Serial.print("OFF: ");
  Serial.println(temp_off_point);
  Serial.print("Count: ");
  Serial.println(change_timer);
  
  char tempString[10]; //Used for sprintf
    
  // if dial was changed show the dial
  if(pentValue != last_pent) {
    last_pent = pentValue; 
    change_timer = 0;
  }
  
  int display_int = 0;
  
  change_timer++;
  if(change_timer < change_timeout) {
    digitalWrite(nob_led_pin, HIGH);
    display_int = pentValue;   
  } else {
    change_timer = change_timeout;
    display_int = tempi;
    digitalWrite(nob_led_pin, LOW);    
  }
  
  sprintf(tempString, "%4d", display_int);
  
  if(tempi < temp_on_point) {
    digitalWrite(relay_pin, HIGH); 
    Serial.println("HEAT: ON");
  } else if(tempi > temp_off_point) {
    digitalWrite(relay_pin, LOW);
    Serial.println("HEAT: OFF");
  }
  
  Serial.println("");

  Serial7Segment.print(tempString);
  
  delay(100);
}

float getVoltage(int pin)
{
  return (analogRead(pin) * .004882814); 
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

void setBrightness(byte value)
{
  Serial7Segment.write(0x7A);  // Set brightness command byte
  Serial7Segment.write(value);  // brightness data byte
}
