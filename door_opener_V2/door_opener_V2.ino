#include <Ethernet.h>
#include <SPI.h>

int door1up = 2;
int door1down = 3;
int door2up = 4;
int door2down = 5;

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0F, 0x92, 0x43};
// assign an IP address for the controller:
IPAddress ip(192,168,1,103);
IPAddress gateway(192,168,1,1);	
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(80);

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
  
  server.begin();
  
  pinMode(door1up, OUTPUT);     
  pinMode(door2up, OUTPUT);     
  pinMode(door1down, OUTPUT); 
  pinMode(door2down, OUTPUT); 
  
  delay(1000);  
  
  Serial.print("Server is up at: ");
  Serial.println(Ethernet.localIP());
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
       //   Serial.print(c);
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


