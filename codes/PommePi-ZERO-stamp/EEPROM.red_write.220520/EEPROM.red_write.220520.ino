#include "EEPROM.h"
int addr = 0; // the current address in the EEPROM 
#define EEPROM_SIZE 32

void setup()
{
  Serial.begin(9600);while(!Serial);Serial.println();
  Serial.println("start...");
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  Serial.println(" bytes read from Flash . Values are:");
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
  }
  Serial.println();
  Serial.println("writing random n. in memory");
}

void loop()
{

  int val = byte(random(10020));
  // write the value to the appropriate byte of the EEPROM.
  // these values will remain there when the board is
  // turned off.
  EEPROM.write(addr, val);
  Serial.print(val); Serial.print(" ");

  addr = addr + 1;
  if (addr == EEPROM_SIZE)
  {
    Serial.println();  addr = 0;
    EEPROM.commit();
    Serial.print(EEPROM_SIZE);
    Serial.println(" bytes written on Flash . Values are:");
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
      Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
    }
    Serial.println(); Serial.println("----------------------------------");
  }
  delay(200);
}
