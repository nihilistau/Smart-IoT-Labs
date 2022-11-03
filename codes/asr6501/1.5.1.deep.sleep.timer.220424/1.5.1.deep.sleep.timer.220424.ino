#include "Arduino.h"
#include "LoRaWan_APP.h"
#define timetillsleep 5000
#define timetillwakeup 15000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;

void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}

void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}

void setup() {
  Serial.begin(9600);
  Radio.Sleep( );  // LoRa modem sleep mode
  TimerInit( &sleep, onSleep );
  TimerInit( &wakeUp, onWakeUp );
  onSleep();
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
