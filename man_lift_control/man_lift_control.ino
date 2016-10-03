// PIN DEFINITIONS----------------//
#define RAISE_RELAY 6
#define LOWER_RELAY 7
#define FORWARD_RELAY 5
#define REVERSE_RELAY 8
#define LEFT_RELAY 16
#define RIGHT_RELAY 12
#define SLOW_RELAY 4
#define PUMP_RELAY 3

const int JOYSTICK_H = A1; 
const int JOYSTICK_V = A0;
#define JOYSTICK_BUTTON 2

#define RED_LED 9
#define YELLOW_LED 10
#define GREEN_LED 11

// MODE TYPES -------------------//
const int RAISELOWER = HIGH;
const int FORWARDBACK = LOW;

#define PRESS_DEBOUNCE_TIME 500

// RUNNIG VARIABLES ----------------//
volatile int mode = RAISELOWER;
volatile unsigned long lastpress = 0;

int j_h = 0;
int j_v = 0;


void setup() {
  Serial.begin(9600);
  
  pinMode(RAISE_RELAY, OUTPUT);
  pinMode(LOWER_RELAY, OUTPUT);
  pinMode(FORWARD_RELAY, OUTPUT);
  pinMode(REVERSE_RELAY, OUTPUT);
  pinMode(LEFT_RELAY, OUTPUT);
  pinMode(RIGHT_RELAY, OUTPUT);
  pinMode(SLOW_RELAY, OUTPUT);
  pinMode(PUMP_RELAY, OUTPUT);

  // HIGH IS OFF
  digitalWrite(RAISE_RELAY, HIGH);
  digitalWrite(LOWER_RELAY, HIGH);
  digitalWrite(FORWARD_RELAY, HIGH);
  digitalWrite(REVERSE_RELAY, HIGH);
  digitalWrite(LEFT_RELAY, HIGH);
  digitalWrite(RIGHT_RELAY, HIGH);
  digitalWrite(SLOW_RELAY, HIGH);
  digitalWrite(PUMP_RELAY, HIGH);

  pinMode(JOYSTICK_BUTTON, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  
  attachInterrupt(0, joybutton, FALLING);
  
  delay(1000);
}

void loop() {
  j_h = analogRead(JOYSTICK_H);
  j_v = analogRead(JOYSTICK_V);
  
  if(mode == RAISELOWER) { // MODE IS RIASE
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);

    
    digitalWrite(FORWARD_RELAY, HIGH);
    digitalWrite(REVERSE_RELAY, HIGH);
    digitalWrite(SLOW_RELAY, HIGH);
    digitalWrite(LEFT_RELAY, HIGH);
    digitalWrite(RIGHT_RELAY, HIGH);
    
    if(j_v > 600) { // RAISE
      digitalWrite(RAISE_RELAY, LOW);
      digitalWrite(PUMP_RELAY, LOW);
      digitalWrite(LOWER_RELAY, HIGH);
    } else if(j_v < 480) { // LOWER
      digitalWrite(PUMP_RELAY, HIGH);
      digitalWrite(RAISE_RELAY, HIGH);
      digitalWrite(LOWER_RELAY, LOW); 
    } else {  // HOLD
      digitalWrite(PUMP_RELAY, HIGH);
      digitalWrite(RAISE_RELAY, HIGH);
      digitalWrite(LOWER_RELAY, HIGH); 
    }
  } else {   // MODE IS MOVE
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW); 
    
    digitalWrite(RAISE_RELAY, HIGH);
    digitalWrite(LOWER_RELAY, HIGH); 

    if(j_v > 980 || j_v < 80) { // FAST
       digitalWrite(YELLOW_LED, LOW);
       digitalWrite(SLOW_RELAY, HIGH); 
    } else { // SLOW
       digitalWrite(YELLOW_LED, HIGH);
       digitalWrite(SLOW_RELAY, LOW); 
    }
    
    if(j_v > 600) { // FORWARD
      digitalWrite(FORWARD_RELAY, LOW);
      digitalWrite(REVERSE_RELAY, HIGH);
      digitalWrite(PUMP_RELAY, LOW);
    } else if (j_v < 480) { // REVERSE
      digitalWrite(FORWARD_RELAY, HIGH);
      digitalWrite(REVERSE_RELAY, LOW);
      digitalWrite(PUMP_RELAY, LOW);      
    } else { // HOLD
      digitalWrite(FORWARD_RELAY, HIGH);
      digitalWrite(REVERSE_RELAY, HIGH);
      digitalWrite(PUMP_RELAY, HIGH);         
    }
    
    if(j_h > 600) {
      digitalWrite(LEFT_RELAY, LOW);
      digitalWrite(RIGHT_RELAY, HIGH); 
    } else if(j_h < 480) {
      digitalWrite(LEFT_RELAY, HIGH);
      digitalWrite(RIGHT_RELAY, LOW); 
    } else {
       digitalWrite(LEFT_RELAY, HIGH);
      digitalWrite(RIGHT_RELAY, HIGH); 
    }
  }
  
  Serial.print("H: ");
  Serial.print(j_h);
  Serial.print("  V: ");
  Serial.print(j_v);
  Serial.print("  Mode: ");
  Serial.println(mode);
}

void joybutton() {
  unsigned long rn = millis();
  if((rn - lastpress) > PRESS_DEBOUNCE_TIME) {
    mode = !mode;
    lastpress = rn;
  }
}
