int pirPin = 2; //digital 2

void setup(){
 Serial.begin(9600); 
 pinMode(pirPin, INPUT);
 delay(5000);
}


void loop(){
  int pirVal = digitalRead(pirPin);

  if(pirVal == LOW){ //was motion detected
    Serial.println("Motion Detected"); 
    delay(2000); 
  }

}
