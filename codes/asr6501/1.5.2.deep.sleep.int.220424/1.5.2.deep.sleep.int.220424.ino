#include "Arduino.h"
#include "LoRa_APP.h"

#define INT_GPIO USER_KEY
#define timetillsleep 5000
static TimerEvent_t sleep;
uint8_t lowpower=1;

void onSleep()
{
  Serial.printf("Going into lowpower mode. Press user key to wake up\r\n");
  delay(5);
  lowpower=1;
}
void onWakeUp()
{
  delay(10);
  if(digitalRead(INT_GPIO) == 0)
  {
    Serial.printf("Woke up by GPIO, %d ms later into lowpower mode.\r\n",timetillsleep);
    lowpower=0;
    //timetillsleep ms later into lowpower mode;
    TimerSetValue( &sleep, timetillsleep );
    TimerStart( &sleep );
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(INT_GPIO,INPUT_PULLUP);
  attachInterrupt(INT_GPIO,onWakeUp,FALLING);
  TimerInit( &sleep, onSleep );
  Serial.printf("Going into lowpower mode. Press user key to wake up\r\n");
  delay(5);
}

int i=0;

void loop() {
  if(lowpower){
    lowPowerHandler();
  }
  Serial.print('.');i++;
  if(i==40) { i=0; Serial.println();}
  delay(400);
}
