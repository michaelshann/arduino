// Designed for use with the YUMO E6B2-CWZ3E Rotary Encoder
//  Wiring:
//    blue - ground
//    shield - ground
//    brown - 5v
//    white - digigtal in 2
//    black - digital in 3 -- not used in current example
//
//   to double the resolution you can uncomming the encoderpin2 lines and hook up
//   black to digital in 3 and switch the rpm calculations to the other line
// 


//these pins can not be changed 2/3 are special pins
int encoderPin1 = 2;
// int encoderPin2 = 3;

unsigned int rpm = 0;
unsigned int ticks = 0;
unsigned long last_time;
unsigned const int delay_time = 500;

void setup() {
  Serial.begin (9600);

  pinMode(encoderPin1, INPUT); 
//pinMode(encoderPin2, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
//digitalWrite(encoderPin2, HIGH);
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
 // attachInterrupt(1, updateEncoder, CHANGE);

}

void loop(){
  unsigned long now = millis();
  if(ticks) {
    rpm = (int)(((60000 / (now - last_time)) * ticks) / 1024);
    //    rpm = (int)(((60000 / (now - last_time)) * ticks) / 2048);
  } else {
    rpm = 0;
  }
  last_time = now;
  ticks = 0;
  Serial.println(rpm);
  delay(delay_time); 
}


void updateEncoder(){
  ticks++;
}
