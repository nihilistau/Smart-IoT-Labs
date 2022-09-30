/****************************************************************
 * ReadSHT2x
 *  An example sketch that reads the sensor and prints the
 *  relative humidity to the PC's serial port
 *
 *  Tested with:
 *    - SHT21-Breakout Humidity sensor from Modern Device
 *    - SHT2x-Breakout Humidity sensor from MisensO Electronics
 ***************************************************************/

#include <Wire.h>
#include <Sodaq_SHT2x.h>


void setup()
{
  Wire.begin(12,14);
  Serial.begin(9600);
}

void loop()
{
   Wire.begin(12,14);
  delay(400);
  Serial.print("     Temperature(C): ");
  Serial.println(SHT2x.GetTemperature());
  delay(400);
  Serial.print("Humidity(%RH): ");
  Serial.println(SHT2x.GetHumidity());
  delay(400);

  
  delay(1000);
}
