const int up_out= 10;
const int down_out = 11;

const int up_in = 5;
const int down_in = 4;

// FOR DOUBLE CLICKING DOWN GOES ALL THE WAY DOWN
int last_press = -1;
unsigned long last_press_time = 0;
unsigned long last_press_timeout = 5000;

int last_press_bounce = 10;
int hold_down = false;

void setup() {  
  Serial.begin(9600);
  
  pinMode(up_out,OUTPUT);
  pinMode(down_out,OUTPUT);

  pinMode(up_in, INPUT);
  pinMode(down_in, INPUT);
  
  digitalWrite(up_out,LOW);     
  digitalWrite(down_out,LOW);
  
  delay(1000); // to remove noise at begining
}

void loop() {

  if(hold_down && digitalRead(up_in) != HIGH) {
    lower();  
  } else {
    hold_down = false;
    if(digitalRead(down_in) == HIGH) {
        if(last_press == down_in && last_press_time > last_press_bounce && last_press_time < last_press_timeout) { // SECOND PRESS OF DOUBLE PESS
          hold_down = true;
        }
        lower();
        last_press = down_in;
        last_press_time = 0;
      } else if(digitalRead(up_in) == HIGH) {
        raise();
        last_press = up_in;
        last_press_time = 0;
      } else {
        hold();  
        if(last_press_time <= last_press_timeout) {
          last_press_time++;
        }
      }
  }
}

void raise() {
  Serial.println("RAISE");
  digitalWrite(up_out,HIGH);
  digitalWrite(down_out,LOW);
  }

void hold() {
  digitalWrite(up_out,LOW);
  digitalWrite(down_out,LOW);
  }

void lower() {
  Serial.println("LOWER");
  digitalWrite(up_out,LOW);
  digitalWrite(down_out,HIGH);
  }
