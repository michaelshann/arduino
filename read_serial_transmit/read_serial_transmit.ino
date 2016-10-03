const int upPin = 2;
const int downPin = 3;

void setup() {
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);  
  Serial.begin(9600);
}

int upState = 0;
int downState = 0;
int lastState = 0;

void loop(){
  upState = digitalRead(upPin);
  downState = digitalRead(downPin);
  
  if(upState == HIGH){
      Serial.println('U');
  } else if (downState == HIGH{
      Serial.println('D');
  } else {
      Serial.println('H'); 
  }
  
}
