/*********************************************************************************
 *  
 *  DRYER MONITOR V2 -- UDOO EDITION
 *  
 *  This program is designed to be run on the udoo with a component python program 
 *  running on the linux part of the udoo to inser the data output on the serial to 
 *  the db
 *  
 *  The Data sent to the python will be encoded in a python dictionary format
 *    {
 *      "plenum_temp":1234
 *      "grain_temp":1234
 *      "grain_moisture":123
 *      "main_motor_status":1
 *      "main_belt_status":1
 *      "lt_motor_status":1
 *      "lt_belt_status":0
 *      "st_motor_status":0
 *      "st_belt_status":2
 *    }
 *  
 *  
 *********************************************************************************/

//#include <OneWire.h> 
#include "EmonLib.h"
 
// CONSTANTS
const byte MOTOR_ON = 1;
const byte MOTOR_OFF = 0;
const byte BELT_OK = 1;
const byte BELT_BROKE = 0;
const byte BELT_UNKNOWN = 2;
const int  IRM_LIMIT = 20;
const int MAIN_MOTOR = 1;
const int LT_MOTOR = 2;
const int ST_MOTOR = 3; 

// STATUS VARIABLES
int plenum_temp;  // actual temp x10 to save decimal
byte grain_moisture;  // actual mositure x10 to save decimal
int grain_temp;  // acutal temp x10 to save decimal
byte main_motor;
byte main_belt;
byte t_motor_large_bin;
byte t_motor_small_bin;
byte t_belt_large_bin;
byte t_belt_small_bin;

// PINS
const int green_led = 8;
const int red_led = 9;
const int main_emon_pin = 0;
const int lt_emon_pin = 1;
const int st_emon_pin = 2;
const int main_belt_hall_pin = 3;
const int plenum_temp_pin = 2;

EnergyMonitor main_emon;
EnergyMonitor lt_emon;
EnergyMonitor st_emon;

//OneWire plenum_ds(plenum_temp_pin);  // on digital pin 2

String serial_data = String();

void setup() {
  Serial.begin(9600); 
  
  main_emon.current(main_emon_pin, 111.1); 
  lt_emon.current(lt_emon_pin, 111.1);
  st_emon.current(st_emon_pin, 111.1);
  
  plenum_temp = 1200;  // actual temp x10 to save decimal
  grain_moisture = 130;  // actual mositure x10 to save decimal
  grain_temp = 1000;  // acutal temp x10 to save decimal
  main_motor = 1;
  main_belt = 1;
  t_motor_large_bin = 0;
  t_motor_small_bin = 0;
  t_belt_large_bin = 0;
  t_belt_small_bin = 0;
}
 
void loop() {
  fill_test_data();
  construct_serial_data();
  Serial.println(serial_data);
  delay(500); 
}

void fill_test_data(void) {
  plenum_temp += (int)random(-30,30);  // actual temp x10 to save decimal
  grain_moisture += (int)random(-10,10);  // actual mositure x10 to save decimal
  grain_temp += (int)random(-40,40);  // acutal temp x10 to save decimal
  main_motor = (int)random(0,2);
  main_belt = (int)random(0,3);
  t_motor_large_bin = (int)random(0,2);
  t_motor_small_bin = (int)random(0,2);
  t_belt_large_bin = (int)random(0,3);
  t_belt_small_bin = (int)random(0,3);
  
  if(plenum_temp > 2200 || plenum_temp < 800) {
    plenum_temp = 1600;
  }
  if(grain_temp > 2100 || grain_temp < 600) {
    grain_temp = 1400;
  }
  if(grain_moisture > 160 || grain_moisture < 80) {
    grain_moisture = 140;
  }
}

void construct_serial_data(void) {
  serial_data = "{";
  serial_data += "\"plenum_temp\":";
  serial_data += plenum_temp;
  serial_data += ", \"grain_temp\":";
  serial_data += grain_temp;
  serial_data += ", \"grain_moisture\":";
  serial_data += grain_moisture;
  serial_data += ", \"main_motor_status\":";
  serial_data += main_motor;
  serial_data += ", \"main_belt_status\":";
  serial_data += main_motor;
  serial_data += ", \"lt_motor_status\":";
  serial_data += t_motor_large_bin;
  serial_data += ", \"lt_belt_status\":";
  serial_data += t_belt_large_bin;
  serial_data += ", \"st_motor_status\":";
  serial_data += t_motor_small_bin;
  serial_data += ", \"st_belt_status\":";
  serial_data += t_belt_small_bin;
  serial_data += "}";
}

// GIVEN A MOTOR TO GET THE STATUS OF RETURN THE STATUS OF THE MOTOR
byte beltStatus(int belt) {
   int irms;
   byte ret_val = MOTOR_OFF;
  
   switch(belt) {
     case MAIN_MOTOR:
       irms = (int)main_emon.calcIrms(1480);
       break;
     case LT_MOTOR:
       irms = (int)lt_emon.calcIrms(1480);
       break;
     case ST_MOTOR:
       irms = (int)st_emon.calcIrms(1480);
       break;
     default:
       irms = 0;
       break;
  }
 
  if(irms > IRM_LIMIT) // MOTOR IS ON
  {
     ret_val = MOTOR_ON;
  } else {  // MOTOR IS OFF
     ret_val = MOTOR_OFF;
  }
  
  return ret_val;
}


// get the temp off the DS18S20
/*
float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !plenum_ds.search(addr)) {
      //no more sensors on chain, reset search
      plenum_ds.reset_search();
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

  plenum_ds.reset();
  plenum_ds.select(addr);
  plenum_ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = plenum_ds.reset();
  plenum_ds.select(addr);    
  plenum_ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = plenum_ds.read();
  }
  
  plenum_ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return ((TemperatureSum * 1.8) + 32.0);
  
}
*/

