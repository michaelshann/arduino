#include <SoftwareSerial.h>
#define COMMAND_TIMEOUT 2000 // ms

const byte XB_RX = 9; // XBee's RX (Din) pin
const byte XB_TX = 8; // XBee's TX (Dout) pin
SoftwareSerial xB(XB_TX,XB_RX);   // (rx,tx)
const int XBEE_BAUD = 9600; // Your XBee's baud (9600 is default)

String WIFI_SSID = "home";
enum encryption{NO_SECURITY, WPA_TKIP, WPA2_AES, WEP};
encryption WIFI_EE = NO_SECURITY;
String WIFI_PSK = "";
String destIP = "173,194,33,104"; //  "192.99.154.218"; // my ip

int temp = 0;
int setTemp = 0;
int tempStatus = 0;

void setup()
{
  xB.begin(XBEE_BAUD);
  Serial.begin(9600);
  connectWiFi(WIFI_SSID, WIFI_EE, WIFI_PSK);
}

void loop()
{
 delay(1000);
  commandMode(0);
 testInternet();
// fillTestData();
// sendData();
}

int testInternet() {
  xB.flush();
  
  xB.println("GET /search?q=arduino HTTP/1.0");
  xB.println();
  
  delay(10);
  char t = ' ';
  
 // we were good little programmers we'd check the content of
  // the OK response. If we were good little programmers...
  char response[12];
  if (waitForAvailable(12) > 0)
  {
    for (int i=0; i<12; i++)
    {
      response[i] = xB.read();
    }
    if (memcmp(response, "HTTP/1.1 200", 12) == 0)
      return 1;
    else
    {
      Serial.println(response);
      return 0; // Non-200 response
    }
  }
  else // Otherwise timeout, no response from server
    return -1;
  
}

void fillTestData() {
  temp = random(50,80);
  setTemp = random(50,80);
  tempStatus = random(0,1); 
}

int sendData() {
  String data = String(80);
  char response[12];
  
  xB.flush();
   
  data = "thermometer_id=1";
  data += "&temperature=";
  data += temp;
  
  xB.println("POST /Temperatures/insert HTTP/1.1");
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

  //xB.flush();
  
Serial.println("Seent Data: ");
Serial.println(data);
Serial.println("Response: ");
  
  delay(5);
  
  
  char t = ' ';
  if (xB.available() > 0)
  {
    for (int i=0; i<12; i++)
    {
      t = xB.read();
Serial.print(t);
      response[i] = t;
    }  
  }
  
  Serial.println("");
  Serial.println("Done");
 
  if (memcmp(response, "HTTP/1.1 200", 12) == 0)
  {
//    Serial.println("Got OK Back");
    return 1;
  } else {
//    Serial.println("Didn't Get OK Back");
    return 0; 
  }
  
  
}

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
    Serial.print('.');
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
Serial.print(".");
    delay(2000);
  }
  Serial.println(".");
}

// Check if the XBee is connected to a WiFi network.
// This function will send the ATAI command to the XBee.
// That command will return with either a 0 (meaning connected)
// or various values indicating different levels of no-connect.
byte checkConnect(String id)
{
  char temp[2];
  commandMode(0);
  while (!commandMode(1))
    ;
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
    char c;
    xB.print("+++");   // Send CMD mode string
    waitForAvailable(1);
    if (xB.available() > 0)
    {
      c = xB.read();
      
      if (c == 'O') // That's the letter 'O', assume 'K' is next
        return 1; // IF we see "OK" return success
    }
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
