#include "EEPROM.h"

#include "Wire.h"
#include "I2C_eeprom.h"

int addr = 0;

I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC256);

uint32_t start, diff, totals = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin(12,14);

  ee.begin();
  if (! ee.isConnected())
  {
    Serial.println("ERROR: Can't find eeprom\nstopped...");
    while (1);
  }

  Serial.print("I2C eeprom library: ");
  Serial.print(I2C_EEPROM_VERSION);
  Serial.println("\n");

  Serial.println("\nTEST: determine size");
  start = micros();
  uint32_t size = ee.determineSize();
  diff = micros() - start;
  Serial.print("TIME: ");
  Serial.println(diff);
  if (size > 0)
  {
    Serial.print("SIZE: ");
    Serial.print(size);
    Serial.println(" Bytes");
  } else if (size == 0)
  {
    Serial.println("WARNING: Can't determine eeprom size");
  }
  else
  {
    Serial.println("ERROR: Can't find eeprom\nstopped...");
    while (1);
  }

  uint8_t b=0x00,d=0xFF;
  
  Serial.println("Write to external EEPROM:");
  for(unsigned int i=0;i<64;i++) 
     { d=random(0,255);ee.writeByte(i,d); }
     
  Serial.println("Bytes in external EEPROM:");
  for(unsigned int i=0;i<64;i++) 
    {  b=ee.readByte(i);Serial.print(b,HEX); }

  if(!EEPROM.begin(64))
  {
    Serial.println("failed to initialise internal EEPROM"); delay(1000000);
  }
  Serial.println();
  
  Serial.println("Bytes in internal EEPROM:");
  for (int i = 0; i<64; i++)
  {
    Serial.print(byte(EEPROM.read(i)),HEX);
  }
  Serial.println();
  
  Serial.println("Write to internal EEPROM:");
  for (int i=0;i<64;i++)
  {
    EEPROM.write(i,random(0,255));  // write to EEPROM buffer
  }
  EEPROM.commit();  // send the EEPROM buffer to memory

  Serial.println("Read from internal EEPROM:");
  for (int i = 0; i<64; i++)
  {
    Serial.print(byte(EEPROM.read(i)),HEX);
  }
  Serial.println();
}


void loop() { }
