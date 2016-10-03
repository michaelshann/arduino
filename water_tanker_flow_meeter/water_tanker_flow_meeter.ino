// reading liquid flow rate using Seeeduino and Water Flow Sensor from Seeedstudio.com
// Code adapted by Charles Gantt from PC Fan RPM code written by Crenn @thebestcasescenario.com
// http:/themakersworkbench.com http://thebestcasescenario.com http://seeedstudio.com
 
const int front_fill_switch = 8;
const int back_fill_switch = 9;
 
boolean front_full = false;
boolean back_full = false;
 
volatile int ticks = 0;  //measuring the rising edges of the signal
unsigned long tps = 0;             // ticks per second * 1000 for decimal
int gpm;                 // gallons per miniute
int hallsensor = 2;      //The pin location of the sensor
unsigned long last_time = 0;      // last time we ran in ms
unsigned long now = 0;            // time now in ms
unsigned long time = 0;           // time between now and last time in ms
unsigned long total_g = 0;        // total gallons * 1000
int b = -1;
int sum = 0;

int whole_gpm = 0;
int whole_total = 0;
int tenth_gpm = 0;
int tenth_total = 0;

const long update_rpm_interval = 1000; // x1ms time between rpm updates counts ticks 
long last_rpm_time = 0;                    //   in that time so you need to wait some to get enough ticks to count

float calibrate = 1.115;
 
void rpm ()     //This is the function that the interupt calls 
{ 
  ticks++;  //This function measures the rising and falling edge of the hall effect sensors signal
} 
// The setup() method runs once, when the sketch starts
void setup() //
{ 
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  Serial.begin(9600);
  
  pinMode(front_fill_switch, INPUT_PULLUP);
  pinMode(back_fill_switch, INPUT_PULLUP);
  
  digitalWrite(front_fill_switch, HIGH);
  digitalWrite(back_fill_switch, HIGH);
  
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
} 
// the loop() method runs over and over again,
// as long as the Arduino has power
void loop ()    
{
  
  if(digitalRead(front_fill_switch) == LOW) {
     front_full = true; 
  } else {
     front_full = false; 
  }
  
  if(digitalRead(back_fill_switch) == LOW) {
     back_full = true; 
  } else {
     back_full = false; 
  }
  
  if((millis() - last_rpm_time) >= update_rpm_interval) {
    cli();      //Disable interrupts
    now = millis();
    last_rpm_time = now;
    time = now - last_time;
    tps = ticks * time; // get ticks per second * 1000
    gpm = (tps * 9) / 20; // calc gpm * 1000
    gpm = (int)((float)gpm * calibrate);
    total_g = total_g + (gpm / (60000 / time));
    last_time = now;
    ticks = 0;   //Set NbTops to 0 ready for calculations
    sei();      //Enables interrupts
    whole_gpm = (int)(gpm / 1000);
    whole_total = (int)(total_g / 1000);
    tenth_gpm = (int)((gpm - (whole_gpm * 1000)) / 100);
    tenth_total = (int)(((int)(total_g - (whole_total * 1000))) / 100);

  }
}

void serialEvent() {
  if(Serial.available() > 0) {
      char in = Serial.read();
      // SEND DATA 
      //   FORMAT: GPM:12.3,TGL:100.3,FF:0,BF:1\n
      if(in == '?') {
        Serial.print("GPM:");
        Serial.print(whole_gpm);
        Serial.print('.');
        Serial.print(tenth_gpm);
        Serial.print(",TGL:");
        Serial.print(whole_total);
        Serial.print('.');
        Serial.print(tenth_total);
        Serial.print(",FF:");
        Serial.print(front_full);
        Serial.print(",BF:");
        Serial.print(back_full);
        Serial.print('\n');
        Serial.flush();
      }
    }
}
