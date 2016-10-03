#include <SoftwareSerial.h>
#include <OneWire.h> 

#define COMMAND_TIMEOUT 2000 // ms

String WIFI_SSID = "home";
enum encryption{NO_SECURITY, WPA_TKIP, WPA2_AES, WEP};
encryption WIFI_EE = NO_SECURITY;
String WIFI_PSK = "";
String destIP = "192.99.154.218"; // data.sparkfun.com's IP address

const byte XB_RX = 9; // XBee's RX (Din) pin
const byte XB_TX = 8; // XBee's TX (Dout) pin
SoftwareSerial xB(3, 2);   // (rx,tx)
const int XBEE_BAUD = 9600; // Your XBee's baud (9600 is default)

const byte LCD_RX = 3; // LCD RX PIN
SoftwareSerial lcd(10,LCD_RX); // 10 unused

int update_timeout = 10000; // update the web ever second = 1000
int last_update_time = 0;

//Temperature chip i/o
int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2
int temp_correction = 0;

void setup()
{
  // Set up serial ports:
  Serial.begin(9600);
  // Make sure the XBEE BAUD RATE matches its pre-set value
  // (defaults to 9600).
  xB.begin(XBEE_BAUD);
  
  lcd.begin(9600);
  clearDisplay();
  
  // Set up WiFi network
  Serial.println("Testing network");
  // connectWiFi will attempt to connect to the given SSID, using
  // encryption mode "encrypt", and the passphrase string given.
  connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
  // Once connected, print out our IP address for a sanity check:
  Serial.println("Connected!");
  Serial.print("IP Address: "); printIP(); Serial.println(); 
  
  lcd.print("Connected");
 
  
  // setupHTTP() will set up the destination address, port, and
  // make sure we're in TCP mode:
  setupHTTP(destIP);
//  Serial.println("Sending Data");
//  sendTestData();
    
}

//////////
// Loop //
//////////
// loop() constantly checks to see if enough time has lapsed
// (controlled by UPDATE_RATE) to allow a new stream of data
// to be posted.
// Otherwise, to kill time, it'll print out the sensor values
// over the serial port.
void loop()
{
  int now = millis();
  float temperature;
  int temp_int;
  
  if((now - last_update_time) > update_timeout) {
Serial.println("Sending Data");
 clearDisplay();
    temperature = getTemp();
    temp_int = toFarh(temperature);
        lcd.print(temp_int);
    sendData(temp_int); 
    last_update_time = now;
  }
  
  
}

int sendData(int temp) {
  String data = String(80);
  char response[12];
  
  xB.flush();
   
  data = "thermometer_id=1";
  data += "&setting=";
  data += "1";
  data += "&temperature=";
  data += temp;
 
  xB.println("POST /thermometers/insert HTTP/1.1");
  xB.println("User-Agent: Arduino/1.1");
  xB.println("Host: www.shannonfarms.com");
  xB.println("Content-Type: application/x-www-form-urlencoded");
  xB.print("Content-Length: ");
  xB.println(data.length());
  xB.println("Accept-Language: en-us");
  xB.println("Accept-Encoding: text/plain");
  xB.println("Connection: Close");
  xB.println();
  xB.println(data);
  xB.println();
  xB.println();

Serial.println("Seent Data: ");
Serial.println(data);

// if (waitForAvailable(5) > 0)
// { 
//    while(xB.available()) {
//      Serial.print(xB.read()); 
//    }
// }

  if (waitForAvailable(12) > 0)
  {
    for (int i=0; i<12; i++)
    {
      response[i] = xB.read();
    }  
  }
  
  Serial.println(response); 
  
  if (memcmp(response, "HTTP/1.1 200", 12) == 0)
  {
    Serial.println("Got OK Back");
    return 1;
  } else {
    Serial.println("Didn't Get OK Back");
    return 0; 
  }
  
  
}

int sendTestData() {
  xB.flush();
  xB.println("GET /search?q=arduino HTTP/1.0");
  xB.println();
  
  char response[12];
  if (waitForAvailable(12) > 0)
  {
    for (int i=0; i<12; i++)
    {
      response[i] = xB.read();
    }
    if (memcmp(response, "HTTP/1.1 200", 12) == 0)
    {
      Serial.println(response);
      return 1;
    }
    else
    {
      Serial.println(response);
      return 0; // Non-200 response
    }
  }
  else // Otherwise timeout, no response from server
    return -1;
}

///////////////////////////
// XBee WiFi Setup Stuff //
///////////////////////////
// setupHTTP() sets three important parameters on the XBee:
// 1. Destination IP -- This is the IP address of the server
//    we want to send data to.
// 2. Destination Port -- We'll be sending data over port 80.
//    The standard HTTP port a server listens to.
// 3. IP protocol -- We'll be using TCP (instead of default UDP).
void setupHTTP(String address)
{
  // Enter command mode, wait till we get there.
  while (!commandMode(1))
    ;

  // Set IP (1 - TCP)
  command("ATIP1", 2); // RESP: OK
  // Set DL (destination IP address)
  command("ATDL" + address, 2); // RESP: OK
  // Set DE (0x50 - port 80)
  command("ATDE50", 2); // RESP: OK

  commandMode(0); // Exit command mode when done
}

///////////////
// printIP() //
///////////////
// Simple function that enters command mode, reads the IP and
// prints it to a serial terminal. Then exits command mode.
void printIP()
{
  // Wait till we get into command Mode.
  while (!commandMode(1))
    ;
  // Get rid of any data that may have already been in the
  // serial receive buffer:
  xB.flush();
  // Send the ATMY command. Should at least respond with
  // "0.0.0.0\r" (7 characters):
  command("ATMY", 7);
  // While there are characters to be read, read them and throw
  // them out to the serial monitor.
  while (xB.available() > 0)
  {
    Serial.write(xB.read());
  }
  // Exit command mode:
  commandMode(0);
}

//////////////////////////////
// connectWiFi(id, ee, psk) //
//////////////////////////////
// For all of your connecting-to-WiFi-networks needs, we present
// the connectWiFi() function. Supply it an SSID, encryption
// setting, and passphrase, and it'll try its darndest to connect
// to your network.
int connectWiFi(String id, byte auth, String psk)
{
  Serial.print("Connecting");
  const String CMD_SSID = "ATID";
  const String CMD_ENC = "ATEE";
  const String CMD_PSK = "ATPK";
  // Check if we're connected. If so, sweet! We're done.
  // Otherwise, time to configure some settings, and print
  // some status messages:
  int status;
  while ((status = checkConnect(id)) != 0)
  {
    // Print a status message. If `status` isn't 0 (indicating
    // "connected"), then it'll be one of these 
    //  (from XBee WiFI user's manual):
    // 0x01 - WiFi transceiver initialization in progress. 
    // 0x02 - WiFi transceiver initialized, but not yet scanning 
    //        for access point. 
    // 0x13 - Disconnecting from access point. 
    // 0x23 – SSID not configured. 
    // 0x24 - Encryption key invalid (either NULL or invalid 
    //        length for WEP) 
    // 0x27 – SSID was found, but join failed. 0x40- Waiting for 
    //        WPA or WPA2 Authentication 
    // 0x41 – Module joined a network and is waiting for IP 
    //        configuration to complete, which usually means it is
    //        waiting for a DHCP provided address. 
    // 0x42 – Module is joined, IP is configured, and listening 
    //        sockets are being set up. 
    // 0xFF– Module is currently scanning for the configured SSID.
    //
    // We added 0xFE to indicate connected but SSID doesn't match
    // the provided id.
    Serial.print(".");
    Serial.println(status, HEX);

   commandMode(1); // Enter command mode

    // Write AH (2 - Infrastructure) -- Locked in
    command("ATAH2", 2);
    // Write CE (2 - STA) -- Locked in
    command("ATCE2", 2);  
    // Write ID (SparkFun) -- Defined as parameter
    command(CMD_SSID + id, 2);
    // Write EE (Encryption Enable) -- Defined as parameter
    command(CMD_ENC + auth, 2);
    // Write PK ("sparkfun6175") -- Defined as parameter
    command(CMD_PSK + psk, 2);
    // Write MA (0 - DHCP) -- Locked in
    command("ATMA0", 2);
    // Write IP (1 - TCP) -- Loced in
    command("ATIP1", 2);

    commandMode(0); // Exit Command Mode CN
Serial.println(".");
    delay(2000);
  }
}

// Check if the XBee is connected to a WiFi network.
// This function will send the ATAI command to the XBee.
// That command will return with either a 0 (meaning connected)
// or various values indicating different levels of no-connect.
byte checkConnect(String id)
{
  Serial.println(" check connect ");
  char temp[2];
  commandMode(0);
  while (!commandMode(1))
    ;
    Serial.println(" command mode ");
  command("ATAI", 2);
  temp[0] = hexToInt(xB.read());
  temp[1] = hexToInt(xB.read());
  xB.flush();

  if (temp[0] == 0)
  {
    command("ATID", 1);
    int i=0;
    char c=0;
    String atid;
    while ((c != 0x0D) && xB.available())
    {
      c = xB.read();
      if (c != 0x0D)
        atid += c;
    }
    if (atid == id)
      return 0;
    else
      return 0xFE;
  }
  else
  {
    if (temp[1] == 0x13)
      return temp[0];
    else
      return (temp[0]<<4) | temp[1];
  }
}

/////////////////////////////////////
// Low-level, ugly, XBee Functions //
/////////////////////////////////////
void command(String atcmd, int rsplen)
{
  xB.flush();
  xB.print(atcmd);
  xB.print("\r");
  waitForAvailable(rsplen);
}

int commandMode(boolean enter)
{
  xB.flush();

  if (enter)
  {
Serial.println("Entering Command Mode");
    char c;
    xB.print("+++");   // Send CMD mode string
    waitForAvailable(1);
    if (xB.available() > 0)
    {
      Serial.print("Xbee Read: ");
      c = xB.read();
      Serial.println(c);
      if (c == 'O') // That's the letter 'O', assume 'K' is next
        return 1; // IF we see "OK" return success
    }
    Serial.println("Returning Fail");
    return 0; // If no (or incorrect) receive, return fail
  }
  else
  {
    command("ATCN", 2);
    return 1;
  }
}

int waitForAvailable(int qty)
{
  int timeout = COMMAND_TIMEOUT;

  while ((timeout-- > 0) && (xB.available() < qty))
    delay(1);

  return timeout;
}

byte hexToInt(char c)
{
  if (c >= 0x41) // If it's A-F
    return c - 0x37;
  else
    return c - 0x30;
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


// return farheniht temp given celcius
// returns an int from a float but the in is x10 to save one decimal
int toFarh(float cel)
{
  return (int)(((cel * 1.8) + 32.0) * 10);
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

void lineTwo()
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
