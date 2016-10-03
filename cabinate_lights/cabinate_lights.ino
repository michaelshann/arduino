//From bildr article: http://bildr.org/2012/08/rotary-encoder-arduino/

//these pins can not be changed 2/3 are special pins
int encoderPin1 = 0;
int encoderPin2 = 1;

int encoderSwitchPin = 3; //push button switch

int lightsPin = 4;

int bluepin = 5;
int greenpin = 20;
int redpin = 7;

int pushbuttondebounce = 300;
int lastpushbutton = 0;
int rightnow = 0;

int brightness = 0;

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

void setup() {
  Serial.begin (9600);

  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);

  pinMode(bluepin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(redpin, OUTPUT);

  digitalWrite(bluepin, HIGH);
  digitalWrite(greenpin, HIGH);
  digitalWrite(redpin, HIGH);

  pinMode(encoderSwitchPin, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(encoderPin1, updateEncoder, CHANGE); 
  attachInterrupt(encoderPin2, updateEncoder, CHANGE);
  attachInterrupt(encoderSwitchPin, pushButton, RISING);
}

void loop(){ 
  encoderValue = encoderValue > 75 ? 75 : encoderValue;
  encoderValue = encoderValue < 0 ? 0 : encoderValue;
  
  brightness = map(encoderValue, 0, 75, 0, 255);
  
  analogWrite(lightsPin, brightness);
  
  //Serial.println(brightness);
  //delay(100); //just here to slow down the output, and show it will work  even during a delay
}

void pushButton() {
  rightnow = millis();
  if((rightnow - lastpushbutton) > pushbuttondebounce) {
    encoderValue = encoderValue == 0 ? 75 : 0;  // If lights are off turn all the way on, if on at all turn all the way off
  }
  lastpushbutton = rightnow;
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
}
