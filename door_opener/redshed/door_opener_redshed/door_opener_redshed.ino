#include <Ethernet.h>
#include <SPI.h>

int door_up = 7;
int door_down = 8;

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0F, 0x32, 0x22};
// assign an IP address for the controller:
IPAddress ip(192,168,1,101);
IPAddress gateway(192,168,1,1);	
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(80);

String readString; 

void setup() {
  Serial.begin(9600);
  
  // start the Ethernet connection:
  Serial.println("Trying to get an IP address using DHCP");
 // if (Ethernet.begin(mac) == 0) {
  //  Serial.println("Failed to configure Ethernet using DHCP");
    // initialize the ethernet device not using DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
//  }
  
  server.begin();
  
  pinMode(door_up, OUTPUT);     
  pinMode(door_down, OUTPUT); 
  
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
          
          String redirect = "<meta http-equiv=\"refresh\" content=\"0; url=http://arduino.shannonfarms.com/redshed/index.htm";
         
          
        ///////////////////// control arduino pin
          if(readString.indexOf("?open") >0)//checks for on
          {
            Serial.println("Raise");
            raise();
          }
          else if(readString.indexOf("?close") >0)//checks for off
          {
            Serial.println("Lower");
            lower();
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

void raise(void) {
 
    digitalWrite(door_up, HIGH);
    delay(500);
    digitalWrite(door_up, LOW);

}

void lower(void) {
    digitalWrite(door_down, HIGH);
    delay(500);
    digitalWrite(door_down, LOW); 
}


