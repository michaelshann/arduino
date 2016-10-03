//*****************************************************************//
// large_digit_display.ino
//  Code for displaying a number on my homemade 5 digit 6.5 inch led displays
//  
//  The Display is a series of TPIC6B595DWRG4 Shift Registers with backpacked
//   to each letter in series, it uses a 12v power signal and a 5v control signal
//   and a latchpin, serial_in, and clock pin, as well as a ground connected
//   
//  printNumber is a function that takes a unsigned long and prints it using the shiftOut function
//
//
//
//*****************************************************************//

#include <SoftwareSerial.h>

#define NUM_LED_DIGITS 5

SoftwareSerial scale(2, 3, true); // 2 data in, with inverted set to true its a rs-232 inverted signal

// Shift Register Pins for the led displays
int latchPin = 9;
int clockPin = 10;
int dataPin = 8;

//holders for infromation you're going to pass to shifting function
byte digits[NUM_LED_DIGITS];   // digits recived max 10
int in_length = 0;  // lenght of input recived
int testCounter = 0;

// Array of Numbers to Pass to the LED's, each byte is the number abover to display a 0 pass byte 0x3F etc.
                          // 0     1      2     3     4    5     6     7     8     9   
byte digit_library[10] = { 0x3F, 0x06, 0x5b, 0x4f, 0x66, 0x6D, 0x7d, 0x07, 0x7f, 0x67};
byte blank_digit = 0x00;
byte negative_sign = 0x40;

void setup() {
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Begining");
  scale.begin(9600);
}

void loop() {
  resetDigits();
  getWeight();
  //testWeight();
  printNumber(); 
}

//*******************************************************************/
// testWeight()
//  incrament a test weight to test the display
//
/********************************************************************/
void testWeight() {
  char buf [10];
  sprintf (buf, "%10i", testCounter);
  for(int i = 0; i < 10; i++) {
    digits[i] = buf[i] - '0';
  }
  testCounter++; 
  delay(10);
}


/********************************************************************/
// getWeight()
//  Wait for serial data to come in on SoftwareSerial device scale 
//   default pin 2, and inverted serial data, data should be in the form
//   "    123\r\n" - preceded by blank spaces and terminated with a /r/n
//   the function returns an unsinged long of the number recieved.
//   
//
//
/*******************************************************************/

void getWeight() {
  byte in = ' ';
  in_length = 0;
  int counter = 0;
  int received_length = 0;
  Serial.println("Reciving: ");
  while(true) {
    if(scale.available()) {
      in = (byte)scale.read();
       Serial.print(in);
       if(in == '\r') {
         Serial.println("");
         return;
       }
       if(in > 47 && in < 58) { // only if its a number
         digits[counter] = digit_library[in - 48];
         counter++;
         in_length++;
       } else if(in == '-') {
         digits[counter] = negative_sign;
         counter++;
         in_length++;
       }
    }
  }
  
}

// Given A number Print it out on the 5 digit 6.5 inch display
void printNumber() {
  digitalWrite(latchPin, LOW);
  // print the number
  for(int g = (in_length - 1); g >= 0; g--) {
    shiftOut(dataPin, clockPin, digits[g]);   
  }
  // print blanks, the right amount of blnaks based on length
  for(int n = 0; n < (NUM_LED_DIGITS - in_length); n++) {
     shiftOut(dataPin, clockPin, blank_digit);
  }
  digitalWrite(latchPin, HIGH);
}

// Resets the digit array over writes everything with the blank digit to erase old data
void resetDigits() {
  for(int x = 0; x < NUM_LED_DIGITS; x++) {
    digits[x] = blank_digit;
  }
}


// the heart of the program
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}
