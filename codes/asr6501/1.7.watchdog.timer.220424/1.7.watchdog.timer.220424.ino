#include "Arduino.h"
#include "innerWdt.h"

// For asr650x, the max feed time is 2.8 seconds.

#define MAX_FEEDTIME 2000 // default is 2800 ms

bool autoFeed = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  Serial.println("Start");

  /* Enable the WDT. 
  * autoFeed = false: do not auto feed wdt.
  * autoFeed = true : it auto feed the wdt in every watchdog interrupt.
  */
  innerWdtEnable(autoFeed);
}
int feedCnt = 0;

void loop() {

  Serial.println("running");
  delay(MAX_FEEDTIME - 100);
  
  if(autoFeed == false)
  {
    //feed the wdt
    if(feedCnt < 3)
    {
      Serial.println("feed wdt");
      feedInnerWdt();
      feedCnt++;
    }
    else
    {
      Serial.println("stop feed wdt");
    }
  }
}
