//********************************************************************************************************//
//  water_tanker_2015.ino        
//      Web Based Controller for filling the water tanker, based on a teensy 3.1 using a Adafruit CC3000
//      for web access.
//
//  Board Layout:
//    Teensy 3.1 Pins: 
//      0: OPEN
//      1: OPEN                            14: Back Button - pullup 10k 
//      2: OPEN                            15: Full Sensor 1 - Low if full - with 8.2k pullup 
//      3: CC3000 - IRQ                    16: Full Sensor 2 - Low if full - with 8.2k pullup 
//      4: CC3000 - VBEN                   17: OPEN
//      5: Relay - Valve 1 Relay A         18: FLow Sensor
//      6: Relay - Valve 1 Relay B         19: Front Button - pullup 10k
//      7: Relay - Valve 2 Relay A         20: Front Light
//      8: Relay - Valve 2 Relay B         21: Back Light
//      9: STATUS LED - RED                22: STATUS LED - GREEN
//      10: CC3000 - CS                    23: STATUS LED - BLUE
//      11: CC3000 - MOSI
//      12: CC3000 - MISO
//      13: CC3000 - CLK
//
//  
//  WEB API: 
//     connects to shannonfarms.com/watertankers/update and sends a put request like:
//      front_full=0&back_full=0&front_switch=0&back_switch=0 // for switches and full status
//     response looks for 
//        front_switch:0; or 1 to turn on ora off front
//        back_switch:0; or 1 to turn on or off back
//        update_time:10000; to set update time to 10000 ms or 10 seconds
//        >> ENDS RESPONSE
//
//********************************************************************************************************//

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include <stdlib.h>
#include "utility/debug.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/************************************* PINS ********************************************/
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  4
#define ADAFRUIT_CC3000_CS    10

#define RELAY_FRONT_1 8
#define RELAY_FRONT_2 7
#define RELAY_BACK_1 6
#define RELAY_BACK_2 5

#define FULL_FRONT 15
#define FULL_BACK 16

#define FLOW_SENSOR 18

#define FRONT_BUTTON 19
#define BACK_BUTTON 14

#define FRONT_LIGHT 20
#define BACK_LIGHT 21

#define STATUS_LIGHT_RED 9
#define STATUS_LIGHT_BLUE 23
#define STATUS_LIGHT_GREEN 22

#define COLOR_RED 1
#define COLOR_BLUE 2
#define COLOR_GREEN 3
#define COLOR_YELLOW 4
#define COLOR_ORANGE 9
#define COLOR_CYAN 5
#define COLOR_MAGENTA 6
#define COLOR_WHITE 7
#define COLOR_BLACK 8

/************************************* END PINS ********************************************/

/************************************* WIFI STUFF ********************************************/
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2);
#define WLAN_SSID       "shannon"
#define WLAN_PASS       "FimmKQH1"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2
//  IP FOR SHANNONFARMS.COM
const uint8_t   SERVER_IP[4]   = { 192, 99, 154, 218 };
#define SERVER_PORT 80

static Adafruit_CC3000_Client client;
static uint8_t connect_retry_count = 0;
#define CONNECT_RETRIES 5

#define WLAN_SETUP_TIMEOUT 10000 // ten seconds default 10000

unsigned long  SERVER_RESPONSE_TIMEOUT = 4000;

/*********************************** END WIFI STUFF ******************************************/

/*********************************** STATUS VAIRABLES ****************************************/

boolean debug = true;

volatile boolean front_full = false;
volatile boolean back_full = false;

volatile boolean front_light = false;
volatile boolean back_light = false;

// The status of the valves
volatile boolean front_valve_status = false; // true = on
volatile boolean back_valve_status = false;

// the switch status weather we want them on or off can be full and on to keep on when not full anymore
volatile boolean front_switch = false;
volatile boolean back_switch = false;

volatile unsigned int front_gal = 0;
volatile unsigned int back_gal = 0;

String response = "";

#define NETWORK_PRE_SETUP 0
#define NETWORK_SETUP_SUCCESS 1
#define NETWORK_CONNECT_SUCCESS 2
#define NETWORK_SETUP_FAIL 3
#define NETWORK_CONNECT_FAIL 4
int network_code = NETWORK_PRE_SETUP; // 0 - power on pre-setup; 1 - setup successfull; 2 - connection successful; 3 - setup fail; 4 - connection fail
/********************************* END STATUS VAIRABLES ****************************************/

/******************************** SHARED RUNNING VAIRABLES **************************************/

volatile unsigned long time_now = 0;
volatile unsigned long last_press = 0;
#define BUTTON_DEBOUNCE 500   // ms between possible button presses

#define BUTTON_BLINK_TIME 500 // HALF SECOND BLINKS
unsigned long front_last_blink = 0;
unsigned long back_last_blink = 0;

unsigned long UPDATE_INTERNET_TIME = 10000; // update ever ten seconds
unsigned long last_update_internet = 0;

/******************************** END SHARED RUNNING VAIRABLES **********************************/

// Set up the HW and the CC3000 module (called automatically on startup)
void setup(void)
{
  if(debug)
    Serial.begin(9600);
  delay(1000); // Without this delay Serial Monitor does not work with teensy 3.1
  
  // SETUPT PINS
  
  // RELAY PINS ALL OUT TO TURN ON OR OFF VALVES
  pinMode(RELAY_FRONT_1, OUTPUT);
  pinMode(RELAY_BACK_1, OUTPUT);
  pinMode(RELAY_FRONT_2, OUTPUT);
  pinMode(RELAY_BACK_2, OUTPUT);  
  
  // FOR FULL SENSORS INPUT IF TANK IS FULL - PULLED LOW ON FULL
  pinMode(FULL_FRONT, INPUT);
  pinMode(FULL_BACK, INPUT);
  
  pinMode(FLOW_SENSOR, INPUT);
  
  pinMode(FRONT_BUTTON, INPUT);
  pinMode(BACK_BUTTON, INPUT);

  pinMode(FRONT_LIGHT, OUTPUT);
  pinMode(BACK_LIGHT, OUTPUT);
  
  pinMode(STATUS_LIGHT_RED, OUTPUT);
  pinMode(STATUS_LIGHT_BLUE, OUTPUT);
  pinMode(STATUS_LIGHT_GREEN, OUTPUT);
  
  setStatusLight(COLOR_YELLOW);
  
  attachInterrupt(FRONT_BUTTON, frontButton, FALLING);
  attachInterrupt(BACK_BUTTON, backButton, FALLING);
  
  // WIFI  
  network_code  = setup_wifi();
}

void loop(void)
{
  unsigned long loop_now = millis();
  if((loop_now - last_update_internet) > UPDATE_INTERNET_TIME && network_code != NETWORK_SETUP_FAIL) {  // if its time and we didn't have an error seting up the wifi run internet stuff
    setStatusLight(COLOR_ORANGE);
    network_code = updateInternet();
    if(network_code != NETWORK_CONNECT_FAIL) {
      network_code = proccessResponse();
    }
    last_update_internet = loop_now;
  }
  runValves();
  runLights();
}

void runLights(void) {  
  
  switch (network_code) {
     case NETWORK_SETUP_SUCCESS:
       setStatusLight(COLOR_BLUE);
       break;
     case NETWORK_CONNECT_SUCCESS:
       setStatusLight(COLOR_GREEN);
       break;
     case NETWORK_SETUP_FAIL:
       setStatusLight(COLOR_RED);
       break;
     case NETWORK_CONNECT_FAIL:
       setStatusLight(COLOR_YELLOW);
       break;
     case NETWORK_PRE_SETUP:
       setStatusLight(COLOR_MAGENTA);
       break;
  }
  
  if(front_full) {
    unsigned long fnow = millis();
    if((fnow - front_last_blink) >= BUTTON_BLINK_TIME) {
      if(debug)
        Serial.println("! - FRONT FULL");
      front_light = !front_light;
      front_last_blink = fnow;
    }
  } else {
    front_light = front_valve_status;
  }
  
  if(back_full) {
    
    unsigned long bnow = millis();
    if((bnow - back_last_blink) >= BUTTON_BLINK_TIME) {
      if(debug)
        Serial.println("! - BACK FULL");
      back_light = !back_light;
      back_last_blink = bnow;   
    } 
  } else {
    back_light = back_valve_status;
  }
  

  digitalWrite(FRONT_LIGHT, front_light);
  digitalWrite(BACK_LIGHT, back_light);
}

void frontButton(void) {
  cli();  // turn off interupts
  time_now = millis();
  
  if((time_now - last_press) > BUTTON_DEBOUNCE) {
    front_switch = !front_switch;
    runValves();
    runLights();
    if(debug)
      Serial.println("* - Front Button");
    last_press = time_now;
  } 
  
  sei();  // turn interupts back on
}

void backButton(void) {
  cli();  // turn off interupts
  time_now = millis();
  
  if((time_now - last_press) > BUTTON_DEBOUNCE) {
    back_switch = !back_switch;
    runValves();
    runLights();
    if(debug)
      Serial.println("* - Back Button");
    last_press = time_now;
  }

  sei();  // turn interupts back on
}


void runValves(void) {
  
  // Check full status
  // opposite of read if HIGH = FALSE, PULLED LOW WHEN FULL
  front_full = !digitalRead(FULL_FRONT);  
  back_full = !digitalRead(FULL_BACK);
  
  // not full && switch only on if full = false and switch = true;
  front_valve_status = !front_full && front_switch; 
  back_valve_status = !back_full && back_switch;


  if(front_valve_status) {
    // OPEN FRONT RELAY_FRONT_1 = HIGH, RELAY_FRONT_2 = LOW
    digitalWrite(RELAY_FRONT_1, HIGH);
    digitalWrite(RELAY_FRONT_2, LOW);
  } else {
    // CLOSE FRONT RELAY_FRONT_1 = LOW, RELAY_FRONT_2 = HIGH
    digitalWrite(RELAY_FRONT_1, LOW);
    digitalWrite(RELAY_FRONT_2, HIGH);     
  }
  
  if(back_valve_status) {
    // OPEN BACK VALVE: RELAY_BACK_1 = HIGH, RELAY_BACK_2 = LOW
    digitalWrite(RELAY_BACK_1, HIGH);
    digitalWrite(RELAY_BACK_2, LOW);
  } else {
    // CLOSE BACK VALVE: RELAY_BACK_1 = LOW, RELAY_BACK_2 = HIGH
    digitalWrite(RELAY_BACK_1, LOW);
    digitalWrite(RELAY_BACK_2, HIGH);    
  }
  
}

void setStatusLight(int color) {
  // CLEAR ANY ANALOG WRITES
  analogWrite(STATUS_LIGHT_RED, 256);
  analogWrite(STATUS_LIGHT_GREEN, 256);
  analogWrite(STATUS_LIGHT_BLUE, 256);
  
  switch (color) {
    case COLOR_RED:
      digitalWrite(STATUS_LIGHT_RED, LOW);
      digitalWrite(STATUS_LIGHT_GREEN, HIGH);
      digitalWrite(STATUS_LIGHT_BLUE, HIGH);
      return;
    case COLOR_BLUE:
      digitalWrite(STATUS_LIGHT_RED, HIGH);
      digitalWrite(STATUS_LIGHT_GREEN, HIGH);
      digitalWrite(STATUS_LIGHT_BLUE, LOW);
      return;
    case COLOR_GREEN:
      digitalWrite(STATUS_LIGHT_RED, HIGH);
      digitalWrite(STATUS_LIGHT_GREEN, LOW);
      digitalWrite(STATUS_LIGHT_BLUE, HIGH);
      return;
    case COLOR_YELLOW:
      digitalWrite(STATUS_LIGHT_RED, LOW);
      analogWrite(STATUS_LIGHT_GREEN, 50);
      digitalWrite(STATUS_LIGHT_BLUE, HIGH);
      return;
    case COLOR_CYAN:
      digitalWrite(STATUS_LIGHT_RED, HIGH);
      digitalWrite(STATUS_LIGHT_GREEN, LOW);
      digitalWrite(STATUS_LIGHT_BLUE, LOW);
      return;
    case COLOR_MAGENTA:
      digitalWrite(STATUS_LIGHT_RED, LOW);
      digitalWrite(STATUS_LIGHT_GREEN, HIGH);
      analogWrite(STATUS_LIGHT_BLUE, 240);
      return;
    case COLOR_WHITE:
      digitalWrite(STATUS_LIGHT_RED, LOW);
      analogWrite(STATUS_LIGHT_BLUE, 196);
      analogWrite(STATUS_LIGHT_GREEN, 156);
      return;
    case COLOR_BLACK:
      digitalWrite(STATUS_LIGHT_RED, HIGH);
      digitalWrite(STATUS_LIGHT_GREEN, HIGH);
      digitalWrite(STATUS_LIGHT_BLUE, HIGH);
      return;
    case COLOR_ORANGE:
      digitalWrite(STATUS_LIGHT_RED, LOW);
      digitalWrite(STATUS_LIGHT_BLUE, HIGH);
      analogWrite(STATUS_LIGHT_GREEN, 200);
      return;      
  }
}

int proccessResponse(void) {
    
    if(debug) {
      Serial.println("--------- RESPONSE ------------");
      Serial.println(response); 
    }
    
  
    if(response.indexOf("200") < 0) {
      if(debug)
        Serial.println("E - DID NOT RECIEVE 200 OK");
      return NETWORK_CONNECT_FAIL;
    }
  
    int fsPOS = response.indexOf("front_switch:");
    int bsPOS = response.indexOf("back_switch:");
    int updatePOS = response.indexOf("update_time:");
    
    if(fsPOS >= 0) {
      char front_switch_internet = response.charAt(fsPOS + 13);
      front_switch = (int)(front_switch_internet - '0');
      if(debug) {
        Serial.print("i - From Internet: Front Switch: ");
        Serial.println(front_switch_internet);
      }
    }
    
    if(bsPOS >= 0) {
      char back_switch_internet =  response.charAt(bsPOS + 12);
      back_switch = (int)(back_switch_internet - '0');
      if(debug)
      {
        Serial.print("i - From Internet: Back Switch: ");
        Serial.println(back_switch_internet);
      }
    }
    
    if(updatePOS >= 0) {
      String updateSubString = response.substring((updatePOS + 12), response.indexOf(';', (updatePOS + 12)));
      UPDATE_INTERNET_TIME = updateSubString.toInt();
      if(debug)
      {
        Serial.print("i - From Internet: Update Time: ");
        Serial.println(UPDATE_INTERNET_TIME);
      }
    }
    
    return NETWORK_CONNECT_SUCCESS;
}




// Tries to read the IP address and other connection details
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}


void connect_to_server(void) 
{
  if(connect_retry_count > 0) {
    if(debug) 
      Serial.println(F("CC3000> Waiting 2 seconds before retrying to connect..."));
    delay(2000);
  }
  if(debug)
    Serial.println(F("CC3000> Connecting to server..."));
  client = cc3000.connectTCP(cc3000.IP2U32(SERVER_IP[0], SERVER_IP[1], SERVER_IP[2], SERVER_IP[3]), SERVER_PORT);
  
  if (!client.connected()) {
    if(debug)
      Serial.println(F("CC3000> Can't connect to server"));
    connect_retry_count++;
    return;
  }
  connect_retry_count = 0;
  if(debug)
    Serial.println(F("CC3000> Connected!"));
}

int setup_wifi(void) {
  /* Initialise the module */
  if(debug)
    Serial.println(F("\nCC3000> Initializing..."));
  
  unsigned long wifi_setup_now;

  if (!cc3000.begin())
  {
    if(debug)
      Serial.println(F("CC3000> Couldn't begin()! Check your wiring?"));
    return NETWORK_SETUP_FAIL;
  }
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    if(debug)
      Serial.println(F("CC3000> Failed!"));
    return NETWORK_SETUP_FAIL;
  }
  
  if(debug)
    Serial.println(F("CC3000> Connected to AP"));
  
  /* Wait for DHCP to complete */
  if(debug)
    Serial.println(F("Request DHCP"));
    
  wifi_setup_now = millis();
  while (!cc3000.checkDHCP())
  {
    delay(100); 
    if((millis() - wifi_setup_now) > WLAN_SETUP_TIMEOUT) {
      // dhcp timout failure
      return NETWORK_SETUP_FAIL;
    }
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  wifi_setup_now = millis();
  while (! displayConnectionDetails()) {
    delay(100);
    if((millis() - wifi_setup_now) > WLAN_SETUP_TIMEOUT) {
      // timout displaying connection details
      return NETWORK_SETUP_FAIL; 
    }
  }
  
  return NETWORK_SETUP_SUCCESS;
}
