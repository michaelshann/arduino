/*
  WiFi Web Server LED Blink
 
 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 9.
 
 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off
 
 This example is written for a network using WPA encryption. For 
 WEP or WPA, change the Wifi.begin() call accordingly.
 
 Circuit:
 * WiFi shield attached
 * LED attached to pin 9
 
 created 25 Nov 2012
 by Tom Igoe
 */
#include <SPI.h>
#include <WiFi.h>

char ssid[] = "shannon";      //  your network SSID (name) 
char pass[] = "FimmKQH1";  // your network password

int status = WL_IDLE_STATUS;
WiFiServer server(80);

int door1up = 2;
int door1down = 3;
int door2up = 4;
int door2down = 5;

IPAddress ip(192, 168, 1, 108);

String readString; 

void setup() {
  Serial.begin(9600);      // initialize serial communication
  
  pinMode(door1up, OUTPUT);     
  pinMode(door2up, OUTPUT);     
  pinMode(door1down, OUTPUT); 
  pinMode(door2down, OUTPUT); 

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    while(true);        // don't continue
  } 

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    //WiFi.config(ip);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  } 
  
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

 if (client) {
    Serial.println("Client Connected");
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string 
          readString += c; 
        //  Serial.print(c);
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
          
          String redirect = "<meta http-equiv=\"refresh\" content=\"0; url=http://www.shannonfarms.com/arduino/grandpashed";
          
          ///////////////////// control arduino pin
          if(readString.indexOf("?open1") >0)//checks for on
          {
            Serial.println("Raise");
            redirect.concat("/south.htm");
            raise(1);
          }
          else if(readString.indexOf("?close1") >0)//checks for off
          {
            Serial.println("Lower");
            redirect.concat("/south.htm");
            lower(1);
          }
          else if(readString.indexOf("?open2") >0)//checks for on
          {
            Serial.println("Raise");
            redirect.concat("/north.htm");
            raise(2);
          }
          else if(readString.indexOf("?close2") >0)//checks for off
          {
            Serial.println("Lower");
            redirect.concat("/north.htm");
            lower(2);
          }
          
           client.println(redirect);
           client.println("\" /></head></html>");
         
           delay(1);
           //stopping client
           client.flush();
           client.stop();
           
           //clearing string for next read
           
           readString="";

        }
      }
    }
  }
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
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void raise(int door) {
  switch (door) {
    case 1:
      digitalWrite(door1up, HIGH);
      delay(500);
      digitalWrite(door1up, LOW);
      break;
    case 2:
      digitalWrite(door2up, HIGH);
      delay(500);
      digitalWrite(door2up, LOW);
      break;
  }
}

void lower(int door) {
  switch (door) {
    case 1:
      digitalWrite(door1down, HIGH);
      delay(500);
      digitalWrite(door1down, LOW); 
      break;
    case 2:
      digitalWrite(door2down, HIGH);
      delay(500);
      digitalWrite(door2down, LOW); 
      break;
  }
}


