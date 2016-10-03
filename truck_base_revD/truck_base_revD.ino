#define UP_OUT 14
#define DOWN_OUT 15
#define TAILGATE_OUT 16 
#define OTHER_OUT 17

#define UP_IN 4
#define DOWN_IN 5
#define TAILGATE_IN 6

#define TIME_TO_HOLD_DOWN 1500
#define TAILGATE_HOLD_DOWN_TIME 5000

boolean up = false;
boolean down = false;
boolean tailgate = false;
boolean released = false;

boolean hold_up = false;
boolean hold_down = false;

byte last_button_press = 0;
byte button_press = 0;
byte button_held_down = 0;
byte tailgate_open = LOW;
unsigned long held_down_time = 0;
unsigned long last_time_check = 0;
unsigned long right_now = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(UP_OUT, OUTPUT);
  pinMode(DOWN_OUT, OUTPUT);
  pinMode(TAILGATE_OUT, OUTPUT);
  pinMode(OTHER_OUT, OUTPUT);
  
  pinMode(UP_IN, INPUT);
  pinMode(DOWN_IN, INPUT);
  pinMode(TAILGATE_IN, INPUT);
  
  delay(1000);
}

void loop() {
  right_now = millis();
  
  // READ THE PINS OR BUTTON IS HELD DOWN
  up = digitalRead(UP_IN);  // WORKS BUT HOLD CAN ONLY BE REMOVED BY PRESSING OTHER BUTTON NOT SAME BUTTON AFTER RELEASE
  down = digitalRead(DOWN_IN);
  tailgate = digitalRead(TAILGATE_IN);
  
  // FOR HOLDING DOWN BUTTONS STICKS AFTER held_down_time > TIME_TO_HOLD_DOWN
  button_press = (up * UP_IN) + (down * DOWN_IN) + (tailgate * TAILGATE_IN);
  held_down_time = (held_down_time + (right_now - last_time_check)) * (button_press == last_button_press);
  button_held_down = (held_down_time > TIME_TO_HOLD_DOWN) * (button_press);
  
  hold_up = (button_press > 0) ? (button_held_down == UP_IN) : hold_up;
  hold_down = (button_press > 0) ? (button_held_down == DOWN_IN) : hold_down;
  
  tailgate_open = ((held_down_time > TAILGATE_HOLD_DOWN_TIME) && (button_press == TAILGATE_IN))  ^ tailgate_open;
  held_down_time = ((held_down_time > TAILGATE_HOLD_DOWN_TIME) && (button_press == TAILGATE_IN) ) ? 0 : held_down_time;
  
  digitalWrite(UP_OUT, (up || hold_up));
  digitalWrite(DOWN_OUT, (down || hold_down));
  digitalWrite(TAILGATE_OUT, tailgate_open); 
//  digitalWrite(OTHER_OUT, (button_held_down > 0));
  
  last_time_check = right_now;
  last_button_press = button_press;
  delay(10);
}
