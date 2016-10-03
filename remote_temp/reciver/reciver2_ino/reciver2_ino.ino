/*
Sainsmart LCD Shield for Arduino
Key Grab v0.2
Written by jacky

www.sainsmart.com

Displays the currently pressed key on the LCD screen.

Key Codes (in left-to-right order):

None   - 0
Select - 1
Left   - 2
Up     - 3
Down   - 4
Right  - 5

*/

#include <LiquidCrystal.h>
#include <DFR_Key.h>

//Pin assignments for SainSmart LCD Keypad Shield
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 
//---------------------------------------------

DFR_Key keypad;

int localKey = 0;
String keyString = "";
 
int inByte = 0;

                 
void setup() 
{ 
  Serial.begin(9600); 
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp v0.2");
  delay(2500);
  
  /*
  OPTIONAL
  keypad.setRate(x);
  Sets the sample rate at once every x milliseconds.
  Default: 10ms
  */
  keypad.setRate(20);

}

void loop() 
{
   if (Serial.available() > 0) {
    inByte = Serial.parseInt();
     
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.setCursor(0, 1);
    lcd.print(inByte);
  }
}
