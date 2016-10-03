const int ledPin = 7;
int incoming;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  // make the pins outputs:
  pinMode(ledPin, OUTPUT);
}

void loop() {
  while (Serial.available() > 0) {
    incoming= Serial.read();
    Serial.print("Something: ");
    Serial.println(incoming);
   // if(incoming == '1') {
      analogWrite(ledPin, HIGH);
    //}
  }
  Serial.println("nothing");
  analogWrite(ledPin, LOW);
}








