#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

// PIN CONSTANTS
static short mosfit1pin = 2;
static short mosfit2pin = 3;
static short lcd_screen = 5;
static short button_rx = 6;
static short button_tx = 4;

// OTHER CONSTANTS
const short FRONT_TANK = 1;
const short BACK_TANK = 2;

// TANK STUFF
boolean front_tank_full = false; // are they full
boolean back_tank_full = false;
boolean front_valve = false;  // is the valve on or off
boolean back_valve = false;
int send_front_valve = -1;    // if we get a on/off from the internet and need to send it to the buttons
int send_back_valve = -1;

// FLOW MEETER DATA
int whole_gpm = 0;
int tenth_gpm = 0;
int whole_total = 0;
int tenth_total = 0;

SoftwareSerial buttons(button_rx, button_tx);

// LCD SCREEN
SoftwareSerial lcd(9,lcd_screen); // pin 9 in unused
int lcd_last_displayed = 0; // what was displayed last time on the lcd screen
const int LCD_GPM = 1;
const int LCD_ERROR = 2;
const int LCD_TANKS_ON = 3;
const int LCD_TANKS_FULL = 4;

// This Will Be an error code set if we hit an error someplace then when updateing lcd it will show that error
int error_code = 0;  
const int FAIL_TO_CONNECT = 1;
const int FAIL_TO_INSERT = 2;

// UPDATE TIMERS AND DELAY TIMERS
const long serial_wait_time = 100;  // x 10ms - how long to wait for a response from other arduino pro micro (default: 1 sec)
const long get_gpm_interval = 1000; // x 1ms - how often to get gpm from other arduino (default: 1 sec)
long last_get_gpm_time = 0;
long upload_data_interval = 300000; // x 1ms - how often to upload data to server (default: 5 minutes)
long last_upload_data_time = 0;
const long update_lcd_interval = 2000; // x 1ms - how often to update lcd (default: 2sec)
long last_update_lcd_time = 0;
const long update_buttons_interval = 100; // x1ms - how often to update the bttons
long last_update_buttons_time = 0;


boolean reading = false;

String readString; 

byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x92, 0x42};
char server[] = "www.shannonfarms.com"; // Google

EthernetClient client;

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  lcd.begin(9600);
  buttons.begin(9600);
  clearResetLCD();
  lcd.print("Getting IP Address");
Serial.println("Getting IP Address");
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    clearResetLCD();
    lcd.print("Error Getting IP");
    // no point in carrying on, so do nothing forevermore:
  } else {
    delay(1000);
    Serial.println("connecting...");
    clearResetLCD();
    lcd.print("IP Address: ");
    selectLineTwo();
    lcd.print(Ethernet.localIP());
  }
  
  pinMode(mosfit1pin, OUTPUT);
  pinMode(mosfit2pin, OUTPUT);
  
  digitalWrite(mosfit1pin, LOW);
  digitalWrite(mosfit2pin, LOW);
  
}

void loop()
{
  
  if((millis() - last_update_buttons_time) >= update_buttons_interval) {
    last_update_buttons_time = millis();
    updateButtons();
  }

  if((millis() - last_get_gpm_time) >= get_gpm_interval) {
    if(getGPM()) {
      last_get_gpm_time = millis();
      Serial.println("Got GPM Data");
      if((millis() - last_upload_data_time) >= upload_data_interval) {
        Serial.println("Uploading Data");
        last_upload_data_time = millis();
        uploadData();
      }
      
      if((millis() - last_update_lcd_time) >= update_lcd_interval) {
        Serial.println("Updating LCD");
        last_update_lcd_time = millis();
        updateLCD();
      }
    }
  }
  
  updateSwitches();
}

  
boolean updateSwitches() {
Serial.println("-------- Valves -------");
 if(front_valve && !front_tank_full) {
Serial.println("Front ON");
   digitalWrite(mosfit1pin, HIGH);
 } else {
Serial.println("Front OFF");
   digitalWrite(mosfit1pin, LOW);
 }
 
 if(back_valve && !back_tank_full)
 {
Serial.println("Black ON");
   digitalWrite(mosfit2pin, HIGH);
 } else {
Serial.println("Black OFF");
   digitalWrite(mosfit2pin, LOW);
 }
 
 return true;
}

boolean updateButtons() {
  String s = "";
  String rec = "";
  
  s += "bf:";
  if(back_tank_full)
    s += '1';
  else
    s += '0';
  s += ",ff:";
  if(front_tank_full)
    s += '1';
  else
    s += '0';
    
    
  // only send the valve info if we need to turn it on or off from the internet
  if(send_front_valve == 1) {
    s += "fn:";  
    s += '1';
    send_front_valve = -1;
  } else if(send_front_valve == 0) {
    s += "fn:";
    s += '0';
    send_front_valve = -1;
  }
  
  if(send_back_valve == 1) {
    s += "bn:";
    s += '1';
    send_back_valve = -1;
  } else if(send_back_valve == 0) {
    s += "bn:";
    s += '0'; 
    send_back_valve = -1;
  }

Serial.println("------ BUTTONS --------");
Serial.print("Sending: ");
Serial.println(s);
  buttons.println(s);
  delay(100);
  
  while (buttons.available()) {
    char inChar = (char)buttons.read(); 
    rec += inChar;
    if (inChar == '\n') {
      break;
    } 
  }
  buttons.flush();

Serial.print("Recived: ");
Serial.println(rec);
  
  if(rec.charAt(0) == '1') {
    front_valve = true;
  } else {
    front_valve = false;
  }
  
  if(rec.charAt(1) == '1') {
    back_valve = true;
  } else {
    back_valve = false;
  }
  
  Serial.println("------ END BUTTONS -------");
  
  return true;
}


// get the GPM from the pro mini via serial communication
// could take up to a second to run
// send format:
//  
boolean getGPM() {
  String in;

  Serial.flush();   // clear any old data
  Serial.write('?');
  delay(10);
  
  while (Serial.available()) {
    char inChar = (char)Serial.read(); 
    in += inChar;
    if (inChar == '\n') {
      break;
    }
  }
  
  int p = in.indexOf("GPM:");
  int e = 0;
  String tmp = "";
  if(p >= 0) {  // we got the GPM:
      e = in.indexOf('.',p);  // first . after GPM:
      p += 4; // where the number actually starts not skpping the GPM:
      tmp = in.substring(p,e);
      whole_gpm = tmp.toInt();
      e += 1; // were the tenth begins
      p = in.indexOf(',',e);
      tmp = in.substring(e,p);
      tenth_gpm = tmp.toInt();
  }
  
  p = in.indexOf("TGL:");
  e = 0;
  tmp = "";
  if(p >= 0) {
    e = in.indexOf('.',p);
    p += 4;
    tmp = in.substring(p,e);
    whole_total = tmp.toInt();
    e += 1;  // where the tenth begins
    p = in.indexOf(',',e);
    tmp = in.substring(e,p);
    tenth_total = tmp.toInt();
  }
  
  p = in.indexOf("FF:");
  e = 0;
  if(p >= 0) {
      p += 3;
      if(in.charAt(p) == '1') {
        front_tank_full = true;
      } else {
        front_tank_full = false;
      }
  }
  
  p = in.indexOf("BF:");
  if(p >= 0) {
    p += 3;
    if(in.charAt(p) == '1') {
      back_tank_full = true; 
    } else {
      back_tank_full = false;
    }
  }
  
  return true;
}

boolean uploadData() { 
   String data = String(80);
   String return_status = "";
   String return_data = "";
  
   // put together data
    data = "gpm=";
    data += whole_gpm;
    data += ".";
    data += tenth_gpm;
    data += "&total=";
    data += whole_total;
    data += ".";
    data += tenth_total;
    data += "&front_full=";
    data += front_tank_full;
    data += "&back_full=";
    data += back_tank_full;
    data += "&front_on=";
    data += front_valve;
    data += "&back_on=";
    data += back_valve;

    Serial.println(data);

  if (client.connect(server, 80)) {
    client.println("POST /water_tankers/insert HTTP/1.1");
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
    delay(100);
  } 
  else {
    Serial.println("connection failed.");
    error_code = FAIL_TO_CONNECT;
    return false;
  }  

  while(client.available() < 1) {;} // wait until we get some data
  
  char c;
  boolean first_line = true;
  boolean reading_data = false;
  while(client.available()) {
    c = client.read();
    
    if(first_line) {
      if(c == '\n')
        first_line = false;
      else 
        return_status += c;    
    }
    
    if(c == '<') 
      reading_data = true;
      
    if(reading_data)
      return_data += c;
      
    if(c == '>')
      reading_data = false;
    
  }

Serial.println("---------- PROCESS RETURN ----------- ");
Serial.print("return status: ");
Serial.println(return_status);
  
  // ERROR DETECTION
  if(return_status.indexOf("200 OK") < 0) {
    error_code = FAIL_TO_CONNECT;
 
  } else {
     if(return_data.indexOf("fail") >= 0) {
        error_code = FAIL_TO_INSERT;
     } else {
       //on:1,off:2

Serial.print("Recived: ");
Serial.println(return_data);
       int pon = return_data.indexOf("on:");
       if(pon >= 0) {
         pon += 3;
         int on =  int(return_data.charAt(pon)) - 48;
Serial.print("ON Recived: ");
Serial.println(on);
         if(on == 1 || on == 3) 
           send_front_valve = 1;
         if(on == 2 || on == 3)
           send_back_valve = 1;
       }
       
       int poff = return_data.indexOf("off:");
       if(poff >= 0) {
         poff += 4;
         int off = int(return_data.charAt(poff)) - 48;
Serial.print("OFF Recived: ");
Serial.println(off);
         if(off == 1 || off == 3) 
           send_front_valve = 0;
         if(off == 2 || off == 3)
           send_back_valve = 0;
       }
       
     }
  }
  
  Serial.println("disconnecting.");
  client.stop();
  return true;
}

void updateLCD() {
  // if there is an error and we didn't display it last time display an error
  if(error_code != 0 && lcd_last_displayed != LCD_ERROR) {
    Serial.println("Printing Errors");
    printError();          // print errors
    lcd_last_displayed = LCD_ERROR; // clear errors
    error_code = 0;
  } else if(lcd_last_displayed == LCD_TANKS_ON && 
          (front_tank_full || back_tank_full)) { // if we displayed tanks on last time show and a tank is full show tanks full
    Serial.println("Printing Tanks Full");
    printTanksFull();
    lcd_last_displayed = LCD_TANKS_FULL;
  } else if(lcd_last_displayed == LCD_GPM) { // last time we showed gpm now show tanks on or off
    Serial.println("Printing Tanks On");
    printTanksOn();
    lcd_last_displayed = LCD_TANKS_ON;
  } else {
    Serial.println("Printing GPM");
    printGPM();
    lcd_last_displayed = LCD_GPM; 
  }
  
}

void printTanksOn() {
  clearResetLCD();
  
  lcd.print("Front Tank:  ");

  if(front_valve)
    lcd.print(" ON");
  else
    lcd.print("OFF");
    
  lcd.print("Back Tank:   ");
  
  if(back_valve)
    lcd.print(" ON");
  else
    lcd.print("OFF");
}

void printTanksFull() {
  clearResetLCD();  
  
  lcd.print("Front Tank: ");

  if(front_tank_full)
    lcd.print("FULL");
  else
    lcd.print("OPEN");
    
  lcd.print("Back Tank:  ");
  
  if(back_tank_full)
    lcd.print("FULL");
  else
    lcd.print("OPEN");
}

void printError() {
  clearResetLCD();
  
  if(error_code == FAIL_TO_CONNECT){
    lcd.print("ERROR CONNECTING TO SERVER");
  } else if(error_code == FAIL_TO_INSERT) {
    lcd.print("ERROR INSERTING INTO DB");
  } else {
    lcd.print("NO Errors Found"); 
  }
  
}

void printGPM() {
  char tmpSting[10];
  clearResetLCD();

  lcd.print("GPM:      ");
  sprintf(tmpSting,"%4u", whole_gpm);          // display the whole number for gpm
  lcd.print(tmpSting);
  lcd.print(".");
  sprintf(tmpSting,"%1u", tenth_gpm);  // display the decimal for gpm
  lcd.print(tmpSting);
  
  selectLineTwo();
  
  lcd.print("Ttl Gal: ");
  sprintf(tmpSting,"%5u", whole_total);   // display the whole number for total gallons
  lcd.print(tmpSting);
  lcd.print(".");
  sprintf(tmpSting,"%1u", tenth_total);  // display the decimal
  lcd.print(tmpSting);  
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

void selectLineTwo()
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

