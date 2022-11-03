#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <softSerial.h>
softSerial softwareSerial(GPIO5 /*TX pin*/, GPIO3 /*RX pin*/);  // GPIO5 (33) , GPIO3 (8)


#define timetillsleep 5000
#define timetillwakeup 10000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1, highpower=0;

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
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW); 
  softwareSerial.begin(9600); 
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}

void setup()
{
pinMode(Vext, OUTPUT);
digitalWrite(Vext, LOW);
delay(500);
Serial.begin(9600);
Radio.Sleep();
softwareSerial.begin(9600);
delay(1000);
Serial.println("Normal serial init");
TimerInit( &sleep, onSleep );
TimerInit( &wakeUp, onWakeUp );
onSleep();
}

char *ptr, gmt[12],clarg[12], clong[12],dgmt[24],dclarg[24], dclong[24];

void loop()
{
  if(lowpower){
    lowPowerHandler();
  }
  if(highpower)
    {
    if(softwareSerial.available())
      {
      char serialbuffer[256] = {0};
      int i = 0;
      while (softwareSerial.available() && i<256)
        {
        serialbuffer[i] = (char)softwareSerial.read();
        i++;
        }
      serialbuffer[i] = '\0';
      if(serialbuffer[0])
        {
        //Serial.print("Received data from software Serial:");
        Serial.println(serialbuffer);
        ptr=strstr(serialbuffer,"RMC,");
        strncpy(gmt,ptr+4,6); Serial.print("GMT:");Serial.println(gmt);
        ptr=strstr(serialbuffer,",A,");
        strncpy(clarg,ptr+3,10); Serial.print("Larg:");Serial.println(clarg);
        ptr=strstr(serialbuffer,",N,");
        strncpy(clong,ptr+3,10); Serial.print("Long:");Serial.println(clong);
        Serial.println();
        }
      }
      digitalWrite(Vext, HIGH);  
    }
  //digitalWrite(Vext, HIGH);
  delay(4000);
}
