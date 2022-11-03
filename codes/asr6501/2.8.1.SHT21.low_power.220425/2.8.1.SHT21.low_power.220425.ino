#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include "SHT21.h"
SHT21 SHT21;

#define timetillsleep 500
#define timetillwakeup 10000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1, highpower=0;

float t,h;
int dt,dh;

char buff[32];

void getSHT21()
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(50);
    Wire.begin(29,28);
    SHT21.begin();

    t=SHT21.getTemperature();
    h=SHT21.getHumidity();
    Serial.print("Humidity(%RH): ");
    Serial.print(h);
    Serial.print("     Temperature(C): ");
    Serial.println(t);
    dt=(int)((t-(int)t)*100.0);  dh=(int)((h-(int)h)*100.0);
    sprintf(buff,"T:%d.%d, H:%d.%d\n",(int)t,dt,(int)h,dh);
    Serial.println(buff); 
    Wire.end();
    digitalWrite(Vext, HIGH);
}
void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;highpower=0;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}
void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;highpower=1;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Radio.Sleep( );
  TimerInit( &sleep, onSleep );
  TimerInit( &wakeUp, onWakeUp );
  onSleep();
}

void loop() {
  if(lowpower){
    lowPowerHandler();
  }
  if(highpower)
     { getSHT21();highpower=0; }
}

   
