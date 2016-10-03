// reading liquid flow rate using Seeeduino and Water Flow Sensor from Seeedstudio.com
// Code adapted by Charles Gantt from PC Fan RPM code written by Crenn @thebestcasescenario.com
// http:/themakersworkbench.com http://thebestcasescenario.com http://seeedstudio.com
 
#include <SoftwareSerial.h>
 
volatile int ticks = 0;  //measuring the rising edges of the signal
unsigned long tps = 0;             // ticks per second * 1000 for decimal
int gpm;                 // gallons per miniute
int hallsensor = 2;      //The pin location of the sensor
unsigned long last_time = 0;      // last time we ran in ms
unsigned long now = 0;            // time now in ms
unsigned long time = 0;           // time between now and last time in ms
unsigned long total_g = 0;        // total gallons * 1000

float calibrate = 1.115;

SoftwareSerial mySerial(5,4); // pin 4 = TX, pin 5 = RX (unused)
 
void rpm ()     //This is the function that the interupt calls 
{ 
  ticks++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 
// The setup() method runs once, when the sketch starts
void setup() //
{ 
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  mySerial.begin(9600); // set up serial port for 9600 baud
  delay(500);

  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
} 
// the loop() method runs over and over again,
// as long as the Arduino has power
void loop ()    
{
  ticks = 0;   //Set NbTops to 0 ready for calculations
  sei();      //Enables interrupts
  
  char gpmstring[10];
  char gpmdecstring[10];
  char tgstring[10];
  char tgdecstring[10];
  int whole_gpm = (int)(gpm / 1000);
  int whole_total = (int)(total_g / 1000);
  int tenth_gpm = (int)((gpm - (whole_gpm * 1000)) / 100);
  int tenth_total = (int)(((int)(total_g - (whole_total * 1000))) / 100);
  
  mySerial.write(254); 
  mySerial.write(128);
  mySerial.write("GPM:      ");
  sprintf(gpmstring,"%4u", whole_gpm);          // display the whole number for gpm
  mySerial.write(gpmstring);
  mySerial.write(".");
  sprintf(gpmdecstring,"%1u", tenth_gpm);  // display the decimal for gpm
  mySerial.write(gpmdecstring);

  mySerial.write(254); 
  mySerial.write(192);
  mySerial.write("Ttl Gal: ");
  sprintf(tgstring,"%5u", whole_total);   // display the whole number for total gallons
  mySerial.write(tgstring);
  mySerial.write(".");
  sprintf(tgdecstring,"%1u", tenth_total);  // display the decimal
  mySerial.write(tgdecstring); 
  
  delay (1000);
  cli();      //Disable interrupts
  now = millis();
  time = now - last_time;
  tps = ticks * time; // get ticks per second * 1000
  gpm = (tps * 9) / 20; // calc gpm * 1000
  gpm = (int)((float)gpm * calibrate);
  total_g = total_g + (gpm / (60000 / time));
  last_time = now;
}
