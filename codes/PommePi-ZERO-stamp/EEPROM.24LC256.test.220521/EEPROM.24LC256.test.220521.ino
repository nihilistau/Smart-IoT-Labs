#include <Wire.h>    
#define disk1 0x50    //Address of 24LC256 eeprom chip

void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) 
{
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}
 
byte readEEPROM(int deviceaddress, unsigned int eeaddress ) 
{
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}
 
void setup(void)
{
  Serial.begin(9600);
  while(!Serial); Serial.println();
  Wire.begin(7,8);
  delay(2000);
}

byte val=0;
int i=0;

void loop()
{
  unsigned int address;
  address=0;val=0;
  for(i=0;i<16;i++)
  {
    writeEEPROM(disk1, address, val);
    address++;val++;
    delay(100);
  }
  Serial.println();
  address=0;
  for(i=0;i<16;i++)
  {
    Serial.print(readEEPROM(disk1, address), DEC);Serial.print(',');
    address++;
    delay(100);
  }
  Serial.println();
  delay(2000);
}
 
