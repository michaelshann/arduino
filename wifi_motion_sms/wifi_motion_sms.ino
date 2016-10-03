/* Setup shield-specific #include statements */
#include <SPI.h>
#include <Adafruit_CC3000.h>

#include <ccspi.h>
#include <Client.h>

#define WIFI_SSID "shannon"
#define WPA_PASSWORD "FimmKQH1"

const uint8_t   SERVER_IP[4]   = { 192, 99, 154, 218 };
#define SERVER_PORT 80
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define ADAFRUIT_CC3000_IRQ 3
#define ADAFRUIT_CC3000_VBAT 5
#define ADAFRUIT_CC3000_CS 10

#define STATUS_LIGHT_RED 6
#define STATUS_LIGHT_BLUE 7
#define STATUS_LIGHT_GREEN 9

#define COLOR_RED 1
#define COLOR_BLUE 2
#define COLOR_GREEN 3
#define COLOR_YELLOW 4
#define COLOR_ORANGE 9
#define COLOR_CYAN 5
#define COLOR_MAGENTA 6
#define COLOR_WHITE 7
#define COLOR_BLACK 8

#define CONNECT_RETRIES 10
#define SERVER_RESPONSE_TIMEOUT 2000
#define NETWORK_CONNECT_FAIL 0
#define NETWORK_CONNECT_SUCCESS 1

Adafruit_CC3000 cc3k = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);

Adafruit_CC3000_Client client;

String response;
int connect_retry_count = 0;
int debug = true; // true or false to enter debug mode

int dns_timout = 20; // try to get the ip 20 times then timeout

// The number of times to trigger the action if the condition is met
// We limit this so you won't use all of your Temboo calls while testing
int maxCalls = 10;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;
int inputPin = 2;

uint32_t ip;

void setup() {
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(inputPin, INPUT);
  pinMode(STATUS_LIGHT_RED, OUTPUT);
  pinMode(STATUS_LIGHT_BLUE, OUTPUT);
  pinMode(STATUS_LIGHT_GREEN, OUTPUT);
  
  // For debugging, wait until the serial console is connected
  while(!Serial);

  status_t wifiStatus = STATUS_DISCONNECTED;
  
  setStatusLight(COLOR_BLUE);
  
  Serial.println(F("\nInitializing..."));
  if (!cc3k.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1) {
      setStatusLight(COLOR_RED);
      delay(100);
    }
  }
  delay(100);
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WIFI_SSID);
  if (!cc3k.connectToAP(WIFI_SSID, WPA_PASSWORD, WLAN_SEC_WPA2)) {
    Serial.println(F("Failed!"));
    while(1) {
      setStatusLight(COLOR_RED);
      delay(100);
    }
  }
  
  Serial.println(F("Request DHCP"));
  while (!cc3k.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  
    /* Display the IP address DNS, Gateway, etc. */  
  if(debug) {
    while (! displayConnectionDetails()) {
      delay(1000);
    }
  }

  Serial.println("Setup complete.\n");

  setStatusLight(COLOR_GREEN);
}

void loop() {
  int sensorValue = digitalRead(inputPin);
  Serial.println("Sensor: " + String(sensorValue));

  if (sensorValue == LOW) {
      Serial.println("\nTriggered! Sending SMS");
      setStatusLight(COLOR_RED);
      updateInternet();
      delay(5000);
      setStatusLight(STATUS_LIGHT_GREEN);
  }
 delay(25);
}

void connect_to_server(void) {
    client = cc3k.connectTCP(ip, 80);  
}

int updateInternet(void) {
  String putdata = "";
  String putheader = "";
  response = "";
  
  putdata += "hash_key=8x820S08vj02lkjsd0820KkjoKLKiopewnnW9808082lkS0WE8wnwe0S";
  
  if(!client.connected() && connect_retry_count <= CONNECT_RETRIES) {
    connect_to_server();
  } else if (connect_retry_count > CONNECT_RETRIES) {
    return NETWORK_CONNECT_FAIL; 
  }

  putheader += "GET /TextAlerts/send HTTP/1.1\r\n";
  putheader += "Host: shannonfarms.com\r\n";
  putheader += "User-Agent: arduino/0.1\r\n";
  putheader += "Accept-Encoding: text/plain\r\n";
  putheader += "Accept: */*\r\n";
  putheader += "Content-Type: application/x-www-form-urlencoded\r\n";
  putheader += "Content-Length: ";
  putheader += putdata.length();
  putheader += "\r\n\r\n";
  putheader += putdata;
  putheader += "\r\n\r\n";
 
  if(debug) {
    Serial.println("---------- PUT REQUEST ---------");
    Serial.print(putheader);
  }
  
  client.print(putheader);
  
    /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  if(debug) {
    Serial.println("---------- PUT RESPONSE ---------");
  }
  char last_char = ' ';
  unsigned long lastRead = millis();
  while (client.connected() && (millis() - lastRead < SERVER_RESPONSE_TIMEOUT)) {
    while (client.available()) {
      char c = client.read();
      response += c;
      if(debug) {
          Serial.print(c); 
      }
      if(c == '>' && last_char == '>') { // EOF Signal
        client.close(); 
        return NETWORK_CONNECT_SUCCESS;
      }
      lastRead = millis();
      last_char = c;
    }
  }
  client.close(); 
  
  return NETWORK_CONNECT_SUCCESS;
}


bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3k.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3k.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3k.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3k.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3k.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3k.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
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
