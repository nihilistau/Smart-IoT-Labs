#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;
 
void setup(){
  Wire.begin(12,14);   
  Serial.begin(9600);
  lightMeter.begin();
  Serial.println("Running...");
  delay(1000);
}
 
void loop() {
  uint16_t lux = lightMeter.readLightLevel();
  delay(1000);
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);
}
