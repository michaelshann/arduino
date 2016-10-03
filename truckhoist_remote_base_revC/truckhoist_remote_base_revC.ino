int inpin1 = 2;  //  UP PIN
int inpin2 = 3;  //  DOWN PIN
int inpin3 = 4;  //  TAILGATE Remote
int inpin4 = 5;  //  UNUSED

int outpin1 = 11; // Labeled Pin 1, Raise Box Solinoid
int outpin2 = 10; // Labeled Pin 2, Lower Box Solinoid
int outpin3 = 8;  // Labeled Pin 3, Release Tailgate Switch
int outpin4 = 9;  // Labeled Pin 4

// Double Clicking the down button makes the box go down for X seconds
unsigned long time_down = 60000; // ms of time to go down default 60000 = 60 seconds
int last_press = -1; // last button pressed;
unsigned long last_press_time = 0; // the ms of the last time a button was pressed
unsigned long last_press_timeout = 400; // max ms of delay between two button presses default 400ms = .4 sec
unsigned long last_press_bounce = 10;  // for debouncing a double press so we don't get a hold down mistaken for a double press
int hold_down = false; // is hold down currently active, after double press detected this is switch to ture for a time
int button_released = true; // to see if the button got released to prevent just held down
unsigned long time_between_press = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(inpin1,INPUT);
  pinMode(inpin2,INPUT);
  pinMode(inpin3,INPUT);
  pinMode(inpin4,INPUT);
  
  pinMode(outpin1,OUTPUT);
  pinMode(outpin2,OUTPUT);
  pinMode(outpin3,OUTPUT);
  pinMode(outpin4,OUTPUT);
  
  delay(1000);
}

void loop() {
  
  if(digitalRead(inpin1)) { // if up is pressed igrnore all else and just raise and return
    raise();
    last_press = inpin1;
    hold_down = false;
    button_released = false;
    last_press_time = millis();
    return;
  }
  
  if(digitalRead(inpin2)) {
     lower();
     
     if(button_released) {
Serial.println("Pressed Down");
Serial.print("Last Press: ");
Serial.println(last_press);
Serial.print("Last Press Time: ");
Serial.println(last_press_time);
Serial.print("Time Between Presses: ");
Serial.println(time_between_press);
       if( last_press == inpin2 ) {
         time_between_press = millis() - last_press_time;
         if(time_between_press > last_press_bounce) { // This is another press and not just held down 
           if(time_between_press < last_press_timeout) { // it hasn't been too long since last press
             // the last press was the down button
Serial.println("HOLD ON");       
               hold_down = true;  // then hold is on
            } else {
              hold_down = false;  // This loop is reached if you press the down button later which will disable hold down
            }
          }
        }
      }
      
      button_released = false;
      last_press = inpin2;
      last_press_time = millis();
      return;
  }

  button_released = true;
  
  if(hold_down) {
    lower();
    return;
  }
  
  hold();
}


void raise() {
  digitalWrite(outpin1,HIGH);
  digitalWrite(outpin2,LOW);
  }

void hold() {
  digitalWrite(outpin1,LOW);
  digitalWrite(outpin2,LOW);
  }

void lower() {
  digitalWrite(outpin1,LOW);
  digitalWrite(outpin2,HIGH);
  }
