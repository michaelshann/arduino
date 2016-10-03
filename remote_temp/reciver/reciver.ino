
#include <LiquidCrystal.h>
#include <DFR_Key.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remote Temp v0.1");
  delay(2500);
}

void loop() {
  String output = String(0);
  while (Serial.available() > 0) {
    int temp = Serial.read();
    output += String(temp);
    if(Serial.read() == '\n')
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temperature:");
      lcd.setCursor(0, 1);
      lcd.print(output);
      output = String(0);
    }
  }
}








