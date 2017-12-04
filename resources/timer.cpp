#include <Arduino.h>

extern "C" {
#include "user_interface.h"
}
//ESP8266 Maximum upto 7 OS_Timer
os_timer_t Timer1;
os_timer_t Timer2;


void Timer_1_Callback(void *pArg)
{
  Serial.println("Timer 1 Event");
}
void Timer_2_Callback(void *pArg)
{
  Serial.println("Timer 2 Event");
}


void setup() {
  Serial.begin(115200);
  delay(3000);
  // put your setup code here, to run once:
  //=================== Create OS timer1
  os_timer_setfn(&Timer1, Timer_1_Callback, NULL);
  os_timer_arm(&Timer1, 1000, true);

  //=================== Create OS timer2
  os_timer_setfn(&Timer2, Timer_2_Callback, NULL);
  os_timer_arm(&Timer2, 1000, true);


void loop() {
  // put your main code here, to run repeatedly:

}