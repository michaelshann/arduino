#include <genieArduino.h>

const int pot_pin = 0;
int temp = 0;

void setup() {
   genieBegin (GENIE_SERIAL, 9600);  //Serial0
   genieWriteObject(GENIE_OBJ_FORM,0,0);
}

void loop(void) {
   temp = map(analogRead(pot_pin),0,1024,0,100);
   genieWriteObject(GENIE_OBJ_ANGULAR_METER,0, temp);
   delay(50);
}
