#include "Arduino.h"
void setup() {
  Serial.begin(9600);
  delay(100);
}

void loop() {
Serial.println("in the loop");
  uint64_t chipID=getID();delay(100);
  Serial.println();Serial.println();
  Serial.printf("ChipID:%04X%08X\r\n",(uint32_t)(chipID>>32),(uint32_t)chipID);
delay(1000);
}
