#include <SoftwareSerial.h>
#include <OneWire.h> 

SoftwareSerial Serial7Segment(9, 4); //RX pin, TX pin

int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2
int temp_correction = 0;

int encoderPin1 = 2;
int encoderPin2 = 3;

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

int settemp = 500; // default set temp when started is 50 degrees
int swing = 20; // default swing is 2 degress up and down from set temp

int relaypin = 7;

unsigned long timechanged = 0;
unsigned long timeout = 2000; // two seconds

void setup() {
  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
  Serial.begin(9600);  
  Serial7Segment.write(0x77); 
  Serial7Segment.write(4);
  
  pinMode(relaypin, OUTPUT);
  
  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);

  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);
}

void loop()
{
  float temperature = getTemp();
  float farh = toFarh(temperature);
  
  int tempi = (int)(farh * 10) + (temp_correction * 10);
  Serial.println(tempi);
  Serial.println(settemp);
  
  unsigned long now = millis();
  if((now - timechanged) > timeout) {
     lcdPrint(tempi);
  } else {
     lcdPrint(settemp); 
  }
  
  if(tempi < (settemp - swing)) {
     Serial.println("ON");
     digitalWrite(relaypin, HIGH);
  } else if( tempi > (settemp + swing)) {
     Serial.println("OFF");
     digitalWrite(relaypin, LOW); 
  }
  
  delay(10);
}

void lcdPrint(int p) {
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", p); //Convert deciSecond into a string that is right adjusted
  Serial7Segment.print(tempString); //Send serial string out the soft serial port to the S7S
}

float toFarh(float cel)
{
  return ((cel * 1.8) + 32.0);
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

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
  
  if(encoderValue == 4) {
    settemp+=10;
    encoderValue = 0;
  } else if(encoderValue == -4) {
    settemp-=10;
    encoderValue = 0;
  }

  timechanged = millis();
  
  lastEncoded = encoded; //store this value for next time
}

