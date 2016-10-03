const int ledPin = 13; // the pin that the LED is attached to
const unsigned int deadmans_limit = 100 ;

int incomingByte;      // a variable to read incoming serial data into
unsigned int deadmans = 0;


void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    deadmans = 0; // reset dead mans switch we got something
    incomingByte = Serial.read();
    if (incomingByte == 'H') {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
  } else { // Deadmans loop if communication is lost or no input is recieved in 1 sec then do this stuff
     deadmans++; // add one to dead mans switch if we dont recieve anything a while turn off
     if(deadmans > deadmans_limit) {
        digitalWrite(ledPin, LOW);
     }
  }
  delay(10);
}

