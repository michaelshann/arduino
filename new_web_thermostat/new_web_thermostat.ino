
#include <SoftwareSerial.h>
#include <OneWire.h> 

//Temperature chip i/o
int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2
int temp_correction = -7;


static short lcd_screen = 3;
SoftwareSerial lcd(9,lcd_screen); 

unsigned int set_temp = 0;

/* Rotary encoder read example */
#define ENC_A A0  
#define ENC_B A1
#define ENC_PORT PINC
 
void setup()
{
  /* Setup encoder pins as inputs */
  pinMode(ENC_A, INPUT);
  digitalWrite(ENC_A, HIGH);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_B, HIGH);
  
  Serial.begin (9600);
  Serial.println("Start");
  
  lcd.begin (9600);
  clearResetLCD();
  lcd.print("Ready");
  delay(1000);
}
 
void loop()
{
  
  float temperature = getTemp();
  float farh = toFarh(temperature);
  
 static uint8_t counter = 0;      //this variable will be changed by encoder input
 int8_t tmpdata;
 /**/
  tmpdata = read_encoder();
  if( tmpdata ) {
    Serial.print("Counter value: ");
    Serial.println((counter/2), DEC);
    counter += tmpdata;
  }
}
 
/* returns change in encoder state (-1,0,1) */
int8_t read_encoder()
{
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  old_AB |= ( ENC_PORT & 0x03 );  //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}

void setLCDCursor(byte cursor_position)
{
  lcd.write(0xFE);
  lcd.write(cursor_position);  // send the cursor position
}

void clearDisplay()
{
  lcd.write(0xFE);
  lcd.write(0x01); 
}

void selectLineTwo()
{ 
  //puts the cursor at line 0 char 0.
  lcd.write(0xFE); //command flag
  lcd.write(192); //position
}

void clearResetLCD()
{
   clearDisplay();
   setLCDCursor(0); 
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

float toFarh(float cel)
{
  return ((cel * 1.8) + 32.0);
}

