static short button1 = 4;
static short button2 = 2;
static short led1 = 5;
static short led2 = 3;

int brightness1 = 0;    // how bright the LED1 is
int brightness2 = 0;    // how bright led 2 is
int fadeAmount = 1;

static int fade_delay = 30; // ms to puse between fade steps
long last_fade1 = 0; // time (ms) of last fade
long last_fade2 = 0;

static int blink_delay = 1000; // ms to pause between blinks
long last_blink1 = 0;
long last_blink2 = 0;
int led1_status = LOW;
int led2_status = LOW;

boolean valve1_on = false;
boolean valve2_on = false;
boolean tank1_full = false;
boolean tank2_full = false;

static int debounce_time = 500; // ms to take out button debounce
long last_press1 = 0;
long last_press2 = 0;

String inputString = "";         // a string to hold incoming data
boolean serial_recived = false;  // whether the string is complete

void setup() {
  Serial.begin(9600);
  
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
 
  digitalWrite(button1, HIGH);   
  digitalWrite(button2, HIGH); 
  
  inputString.reserve(5);
}

void loop() {
  if(serial_recived) {
    proccessSerial();
    sendSerial();
  }
  
  if(digitalRead(button1) == LOW) { // if the buttons pressed
    if((millis() - last_press1) >= debounce_time) {
      last_press1 = millis();
      if(!tank1_full || valve1_on)                // if the tank is not full or the valve is on already
        valve1_on = !valve1_on;      // then switch the valve
    }
  }
  
  if(digitalRead(button2) == LOW) {
    if((millis() - last_press2) >= debounce_time) {
       last_press2 = millis();
       if(!tank2_full || valve2_on)
         valve2_on = !valve2_on;
    }
  }
  
  if(tank1_full) {  // if fronts full then blink the light and turn vlave off
    blinkLight(1);
    valve1_on = false;
  } else if(valve1_on) {  // if not full and valve on fade light up
    digitalWrite(led1,HIGH);
  } else {              // turn off light if nothing else
    digitalWrite(led1,LOW);
  }
  
  if(tank2_full) {  // if the back tanks full blikn the light and turn the valve off
    blinkLight(2);
    valve2_on = false;
  } else if(valve2_on) {  // if its not full and on fade the light up
    digitalWrite(led2,HIGH);
  } else {                // if nothing then turn the light off
    digitalWrite(led2,LOW);
  }
  
}

void blinkLight(int valve) {
  
  switch(valve) {
    case 1:
     if((millis() - last_blink1) >= blink_delay) {
      led1_status = !led1_status;
      digitalWrite(led1,led1_status);
      last_blink1 = millis(); 
     }
     break;
    case 2:
      if((millis() - last_blink2) >= blink_delay) {
        led2_status = !led2_status;
        digitalWrite(led2,led2_status);
        last_blink2 = millis(); 
       }
     break; 
  }
}

void fadeLight(int valve) {
  switch(valve) {
    case 1:
      if((millis() - last_fade1) >= fade_delay) {
        analogWrite(led1, brightness1);
        brightness1 += fadeAmount;
        if(brightness1 >= 255)
          brightness1 = 0;
        last_fade1 = millis();
      }
      break;
    case 2:
      if((millis() - last_fade2) >= fade_delay) {
        analogWrite(led2, brightness2);      
        brightness2 += fadeAmount;
        if(brightness2 >= 255)
          brightness2 = 0;  
        last_fade2 = millis();
      }
      break;
  }
  
}

// send data back to main board -- button pressed basically is all
// 0 or 1 -- char on or off front valve
// 0 or 1 -- char on or off back valve
// '\n'    -- char done
void sendSerial() {
  if(valve1_on) 
    Serial.print('1');
  else
    Serial.print('0');
  if(valve2_on)
    Serial.print('1');
  else
    Serial.print('0');
    
  Serial.print('\n');
}

/** proccessSerial()
  *   Serial Data read from main to inputString in the form:
  *   bf:1,ff:0,bn:1,fn:0\n -- back full: yes, front full: no, back on: yes, front on: no
  *
  *   back full and front full are always sent, back or front on or off are only sent when a change has come from the internet
***/
void proccessSerial() {
  int pos = 0;
  char r;
  
  pos = inputString.indexOf("bf:");
  if(pos >= 0) {
    r = inputString.charAt(pos + 3);
    if(r == '1') {
      tank2_full = true;
      valve2_on = false;
    } else
      tank2_full = false;
  }
  
  pos = inputString.indexOf("ff:");
  if(pos >= 0) {
    r = inputString.charAt(pos + 3);
    if(r == '1') {
      tank1_full = true;
      valve1_on = false;
    } else
      tank1_full = false;
  }
  
  pos = inputString.indexOf("bn:");
  if(pos >= 0) {
    r = inputString.charAt(pos + 3);
    if(r == '1')
      valve2_on = true;
    else
      valve2_on = false;
  }
  
  pos = inputString.indexOf("fn:");
  if(pos >= 0) {
    r = inputString.charAt(pos + 3);
    if(r == '1')
      valve1_on = true;
    else
      valve1_on = false;
  }
  
  serial_recived = false;  // processed the serial we got so don't come back here till we get more
  inputString = "";
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      serial_recived = true;
    } 
  }
  Serial.flush();
}

void debugStatus() {
  Serial.println("--- DEBUG ---");
  
  Serial.print(" Front Valve: ");
  if(valve1_on)
    Serial.println("On");
  else
    Serial.println("Off");
  
  Serial.print(" Back Valve: ");
  if(valve2_on)
    Serial.println("On");
  else
    Serial.println("Off");
  
  Serial.print(" Front Full: ");
  if(tank1_full)
    Serial.println("Full");
  else
    Serial.println("No");
    
  Serial.print(" Back Full: ");
  if(tank2_full)
    Serial.println("Full");
  else
    Serial.println("No");  
    
 Serial.println("--- END DEBUG ---");
}
