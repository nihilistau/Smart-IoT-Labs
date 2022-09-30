#include <Wire.h>   

void I2Cscan()
{
  byte address;
  int nDevices;
  delay(200);
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
    {
    Wire.beginTransmission(address);
    if(! Wire.endTransmission())  // active address found
      {
      Serial.print("I2C device found at address 0x");
      if (address<16) Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
      }
    }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");   
}               

void setup()
{
  Serial.begin(9600);
  Wire.begin(12, 14, 400000); // SDA, SCL on ESP32, 400 kHz rate
  delay(1000); Serial.println();Serial.println();
  I2Cscan();
  delay(1000);
}
  
void loop()
{  
}
