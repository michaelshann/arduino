int last_hz = 0;

void setup() {
//  Serial.begin(9600);
}

void loop() {
    int hz = analogRead(A0) * 2;
   // Serial.println(hz);
    delay(100);
    if(hz != last_hz) {
      noTone(3);
      tone(3, hz);  
   }
    
   last_hz = hz;

}
