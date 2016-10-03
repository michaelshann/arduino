#include <Ethernet.h>
#include <SPI.h>

int door1up = 3;
int door1down = 8;
int door2up = 6;
ind door2down = 5;

EthernetServer server(80);

String readString; 

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  } 
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
  
  pinMode(doorup, OUTPUT);     
  pinMode(doordown, OUTPUT); 
  
  delay(1000);  
}

void loop() {  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Client Connected");
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string 
          readString += c; 
          Serial.print(c);
        } 

        //if HTTP request has ended
        if (c == '\n') {

          ///////////////
          Serial.println("INCOMING");
          Serial.println(readString); //print to serial monitor for debuging 

      
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();

          client.println("<!DOCTYPE html> <html> <head>");
          
          String intro_redirect = "<meta http-equiv=\"refresh\" content=\"0; url=http://www.shannonfarms.com/arduino/redshed";
          String full_redirect;
          
          int slash = readString.indexOf('/');
          if(slash >=0) {
            String subs = readString.substring(slash);
            subs = subs.substring(0,subs.indexOf(' '));
            
            Serial.print("Slash INDEX: ");
            Serial.println(slash);
            Serial.print("URL: ");
            Serial.println(subs);
            
            full_redirect = intro_redirect + subs + "\"";
            Serial.println(full_redirect);
           }
           
           client.println(full_redirect);
           client.println("</head></html>");
          
          delay(1);
          //stopping client
          client.stop();

          ///////////////////// control arduino pin
          if(readString.indexOf("?open") >0)//checks for on
          {
            Serial.println("Raise");
            raise();
          }
          if(readString.indexOf("?close") >0)//checks for off
          {
            Serial.println("Lower");
            lower();
          }
          
          //clearing string for next read
          readString="";

        }
      }
    }
  }
} 

void raise() {
  digitalWrite(doorup, HIGH);
  delay(500);
  digitalWrite(doorup, LOW); 
}

void lower() {
  digitalWrite(doordown, HIGH);
  delay(500);
  digitalWrite(doordown, LOW);   
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
