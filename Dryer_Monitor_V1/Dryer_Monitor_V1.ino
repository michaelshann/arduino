#include <SoftwareSerial.h>
#include <OneWire.h> 
#include "EmonLib.h"                   // Include Emon Library
#include <SPI.h>
#include <Ethernet.h>
 
SoftwareSerial Serial7Segment(5, 6); //RX pin, TX pin

// Energy Monitor for monitoring current drawn to the motor to check if its on
EnergyMonitor emon; 
int irm_limit = 20;
int emon_pin = 0;

// LEDs
int motor_on_pin = 8;   // led for motor on
int belt_broke_pin = 9;  // led for belt broke
int motor_status;

int belt_switch_pin = 3;  // belt switch pin
int belt_state;  // current state read on belt switch
int belt_last_state;  // last state read to see if change
int belt_status;  // current belt state LOW = broke, HIGH = good
int belt_count = 0;  // number or loops run since last state change on belt
int max_belt_count = 10;  // HOW LONG FOR BELT TIMEOUT 10 = 1 sec

//Temperature chip i/o
int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2

// NETWORK STUFF
int network_counter = 0;
int update_amount = 150; // 150 -> Two Minutes
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient client;


void setup() {
  
  Serial.begin(9600); 
  
  // PINS
  pinMode(motor_on_pin, OUTPUT);  
  pinMode(belt_broke_pin, OUTPUT);
  pinMode(belt_switch_pin, INPUT);
  
  // Prime the belt state to detect change in fist loop
  belt_state = digitalRead(belt_switch_pin);

  // setup energy monitor for decting motor is on
  emon.current(A0, 111.1); 
  
  // SETUP the LCD 
  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  resetLCD();
  
  // NETWORKING
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
}
 
void loop()
{
   // BELT BROKEN DECTION CODE
   belt_last_state = belt_state;
   belt_state = digitalRead(belt_switch_pin);
   
   if(belt_last_state == belt_state) // NO CHANGE
   {
     belt_count++;
     if(belt_count > max_belt_count) // TIMEOUT TO LONG WITHOUT CHANGE THEN BROKEN
     {
        belt_status = LOW;
        Serial.println("BELT OFF");
     }
   } else { // Change
     Serial.println("BELT ON");
     belt_status = HIGH;
     belt_count = 0;
   }
   
  // CURRENT SENSING
  double Irms = emon.calcIrms(1480);  // Calculate Irms only
  Serial.print("IRMS: ");
  Serial.println(Irms);  

  if(Irms > irm_limit) // MOTOR IS ON
  {
     motor_status = HIGH;
     digitalWrite(motor_on_pin,HIGH);  // TURN ON MOTOR ON LED
     if(belt_status) { // CHECK IF BELT IS RUNING
        digitalWrite(belt_broke_pin,LOW);  // BELT IS RUNNING TURN OFF BELT BROKE LED
     } else {
        digitalWrite(belt_broke_pin,HIGH);  // BELT IS NOT RUNNING TURN ON BELT BROKE LED
     } 
  } else {  // MOTOR IS OFF
     motor_status = LOW;
     digitalWrite(motor_on_pin,LOW); // MOTOR IS OFF TURN OFF BOTH LEDS
     digitalWrite(belt_broke_pin,LOW);
  }

  
  // TEMP STUFF
  float temperature = getTemp();
  float farh = toFarh(temperature);
  
  int farteni = (int)(farh * 10);
  int farWhole = farteni / 10;
  int farDec = farteni - (farWhole * 10);
  Serial.print("F: ");
  
  Serial.print(farWhole);
  Serial.print(".");
  Serial.println(farDec);
  
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", farteni); 
  resetLCD();  // RESET LED BEFORE PRINTING
  Serial7Segment.print(tempString); //Send serial string out the soft serial port to the S7S
 
  // NETWORKING
  
  // Build the Post Data to upload
  String data;
  String period = ".";
  data+="";
  data+= "plenum_temp=";
  data+= farWhole;
  data+= period;
  data+= farDec;
  data+= "&motor_status=";
  data+= motor_status;
  data+= "&belt_status=";
  data+= belt_status;
  
  network_counter++; 
  Serial.print("Network Counter: ");
  Serial.println(network_counter);
  
  if(network_counter > update_amount) // don't do this every loop but only after update amount loops
  {
    if (client.connect("www.shannonfarms.com", 80)) {  
      network_counter = 0; // got a connection so don't try again next time
      Serial.println("connected");
      client.println("POST /Dryers/add HTTP/1.1");
      client.println("Host: www.shannonfarms.com");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Connection: close");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println();
      client.print(data);
      client.println();
      
      Serial.println("data: " + data);
      Serial.println("disconnecting.");
      client.stop();
    } 
    else {
      // kf you didn't get a connection to the server:
      Serial.println("connection failed");
    }
  } 
  
  delay(100);
 
  Serial.println("");
}

// clear and reset the LCD 
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
