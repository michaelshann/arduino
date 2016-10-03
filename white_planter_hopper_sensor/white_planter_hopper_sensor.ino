#define LEFT_LIGHT 4
#define RIGHT_LIGHT 5
#define BUZZER 3
#define LEFT_SENSOR 10
#define RIGHT_SENSOR 12
#define MUTE_BUTTON 13
#define MUTE_LIGHT 11

byte left_status = 0;
byte right_status = 0;
byte mute_status = 0;

void setup() {
 Serial.begin(9600);
  
 pinMode(LEFT_LIGHT, OUTPUT);
 pinMode(RIGHT_LIGHT, OUTPUT);
 pinMode(BUZZER, OUTPUT);
 pinMode(MUTE_LIGHT, OUTPUT);
 
 pinMode(MUTE_BUTTON, INPUT);
 pinMode(LEFT_SENSOR, INPUT);
 pinMode(RIGHT_SENSOR, INPUT);
 
 delay(1000); 
}

void loop() {
  // Sensors return high when empty
  //  status = high = empty = light on and buzzer on
  left_status = digitalRead(LEFT_SENSOR);   
  right_status = digitalRead(RIGHT_SENSOR);
  mute_status = digitalRead(MUTE_BUTTON);
  
  Serial.print("L: ");
  Serial.print(left_status);
  Serial.print(" R: ");
  Serial.print(right_status);
  Serial.print(" M: ");
  Serial.println(mute_status);
  
  digitalWrite(MUTE_LIGHT, mute_status);
  digitalWrite(LEFT_LIGHT, left_status);
  digitalWrite(RIGHT_LIGHT, right_status);
  digitalWrite(BUZZER, ((left_status || right_status) && !mute_status));
  
}
