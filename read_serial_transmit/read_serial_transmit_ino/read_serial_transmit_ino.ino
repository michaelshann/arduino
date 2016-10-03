const int buttonPin = 7;

int buttonState = 0;

void setup()
{
   Serial.begin(9600); 
   pinMode(buttonPin,INPUT);
}

void loop()
{
  buttonState = digitalRead(buttonPin);
  
  if(buttonState == HIGH)
  {
     Serial.print("H"); 
  } else if(buttonState == LOW)
  {
     Serial.print("L"); 
  }
  delay(1000);
}
