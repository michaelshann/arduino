#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>

int door1up = 3;
int door1down = 6;
int door2up = 7;
int door2down = 5;

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0F, 0x52, 0x6A};
// assign an IP address for the controller:
IPAddress ip(192,168,1,102);
IPAddress gateway(192,168,1,1);	
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(80);

const int chipSelect = 4;  
Sd2Card card;
File webFile;

String readString; 

void setup() {
  Serial.begin(9600);
  
  // start the Ethernet connection:
  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // initialize the ethernet device not using DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
  }
   // initialize SD card
  Serial.println("Initializing SD card...");
  pinMode(10, OUTPUT);
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  server.begin();
  
  pinMode(door1up, OUTPUT);     
  pinMode(door1down, OUTPUT); 
  pinMode(door2up, OUTPUT);     
  pinMode(door2down, OUTPUT); 
  
  delay(1000);  
}


void loop() {  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string 
          readString += c; 
          //Serial.print(c);
        } 

        //if HTTP request has ended
        if (c == '\n') {

          ///////////////
          Serial.println("INCOMING");
          Serial.println(readString); //print to serial monitor for debuging 

      
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();
          
          int getindex = readString.indexOf("GET");
          Serial.print("Get Index: ");
          Serial.println(getindex);
   
         if(readString.indexOf("/n.htm") >=0) { // if asking for n.htm open N.HTM
            Serial.print("Sending North");
            webFile = SD.open("N.HTM");     
          } else if(readString.indexOf("/w.htm") >=0) { // or if asking for w.htm onpen W.HTM
            Serial.println("Sending West");
            webFile = SD.open("W.HTM");     
          } else if(readString.indexOf(" / ") >=0 ||
                    readString.indexOf("/i.htm") >=0){  // if not asking for anything specific open I.HTM
            Serial.println("Sending Index");
            webFile = SD.open("I.HTM");     
          }
          
          if (webFile) {
              while(webFile.available()) {
                client.write(webFile.read()); // send web page to client
              }
              webFile.close();
           } else {
              Serial.println("ERROR OPENING FILE"); 
          }
          
          delay(1);
          //stopping client
          client.stop();

          ///////////////////// control arduino pin
          if(readString.indexOf("?open1") >0)//checks for on
          {
            Serial.println("Raise One");
            raise(1);
          }
          if(readString.indexOf("?close1") >0)//checks for off
          {
            Serial.println("Lower One");
            lower(1);
          }
          if(readString.indexOf("?open2") >0)//checks for on
          {
            Serial.println("Raise Two");
            raise(2);
          }
          if(readString.indexOf("?close2") >0)//checks for off
          {
            Serial.println("Lower Two");
            lower(2);
          }
          //clearing string for next read
          readString="";

        }
      }
    }
  }
} 

void raise(int door) {
  if(door == 1) {
    digitalWrite(door1up, HIGH);
    delay(500);
    digitalWrite(door1up, LOW); 
  } else if( door == 2) {
    digitalWrite(door2up, HIGH);
    delay(500);
    digitalWrite(door2up, LOW);     
  }
}

void lower(int door) {
  if(door == 1) {
    digitalWrite(door1down, HIGH);
    delay(500);
    digitalWrite(door1down, LOW);   
  } else if(door == 2) {
    digitalWrite(door2down, HIGH);
    delay(500);
    digitalWrite(door2down, LOW);       
  }
}

void listenForEthernetClients() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
   // Serial.println("Got a client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          // print the current readings, in HTML format:
          client.println("Connected");
          client.println("<br />");  
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
} 

