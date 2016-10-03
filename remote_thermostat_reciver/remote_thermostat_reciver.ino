int incomingByte = 0;   // for incoming serial data
int relay_pin = 2;
int current_state = 0; // 0 = off, 1 = on
int standby_pin = 5;

void setup() {
  Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
  pinMode(relay_pin, OUTPUT); 
  pinMode(standby_pin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read(); // READ CURRENT BYTE
    if(incomingByte == 'N' && !current_state) {  // if we get a turn on and its currently off turn it on
       turnOn();
       current_state = 1;
    } else if (incomingByte == 'F' && current_state){
       turnOff();
       current_state = 0;
     }
    Serial.flush();  // CLEAR ANY IF WE MISSED SOME
  } 
}

void turnOff() {
   digitalWrite(standby_pin, HIGH);
   delay(100);
   digitalWrite(standby_pin, LOW); 
}s

void turnOn() {
 ///  Serial.println("Changing State");
   digitalWrite(relay_pin, HIGH);
   delay(100);
   digitalWrite(relay_pin, LOW);
   delay(100);
   digitalWrite(relay_pin, HIGH);
   delay(100);
   digitalWrite(relay_pin, LOW);
}
