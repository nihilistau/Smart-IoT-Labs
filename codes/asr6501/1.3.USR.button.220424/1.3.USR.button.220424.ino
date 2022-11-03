#include "Arduino.h"
#define USR GPIO7  // user button
uint32_t cnt = 0;

void cntIncrease()
{
  cnt++;
  Serial.println(cnt);
}

void setup() {
  Serial.begin(9600);
  PINMODE_INPUT_PULLUP(USR);
  attachInterrupt(USR,cntIncrease,FALLING);
}

void loop() {}
