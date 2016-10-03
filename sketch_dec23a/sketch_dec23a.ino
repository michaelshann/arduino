
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h> 

int DS18S20_Pin = 3; //DS18S20 Signal pin on digital 2
OneWire ds(DS18S20_Pin);  // on digital pin 2
int temp_correction = 0;

int update_time = 360000; //in ms default: 360000 5 minutes

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x0F, 0x92, 0x44 };
IPAddress server(192,99,154,218); // Google
//IPAddress ip(192,168,11,121);

// fill in your Domain Name Server address here:
//IPAddress myDns(8,8,8,8);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  //Ethernet.begin(mac, ip, myDns);
  Ethernet.begin(mac);
  
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  // give the Ethernet shield a second to initialize:
  delay(1000);

}

void loop()
{
  
  float temperature = getTemp();
  float farh = toFarh(temperature);
  int tempi = (int)(farh * 10) + (temp_correction * 10);

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    String data = String(80);

    data = "thermometer_id=1";
    data += "&temperature=";
    data += tempi;
  
    client.println("POST /Temperatures/insert HTTP/1.1");
    client.println("User-Agent: Arduino/1.1");
    client.println("Host: www.shannonfarms.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println("Accept-Language: en-us");
    client.println("Accept-Encoding: text/plain");
    client.println("Connection: Close");
    client.println();
    client.println(data);
    client.println();
    client.println();
    client.println("GET /search?q=arduino HTTP/1.0");
    client.println();
    
    Serial.println("Server Connected");
    Serial.println("Response: ");
    
    delay(1000); // wait for response
    
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    Serial.println(' ');
    client.stop();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  } 

  delay(update_time);  // wait a certain amount of time to update again
}

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
