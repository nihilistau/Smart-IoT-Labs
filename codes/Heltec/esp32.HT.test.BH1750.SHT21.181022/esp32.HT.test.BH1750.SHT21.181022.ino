#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include <BH1750.h>

BH1750 lightMeter;

void setup() {
  Serial.begin(9600);

 Wire.begin(); 
 delay(1000);
 //lightMeter.begin();
 delay(1000);

}


float temp,humi,lumi;
void loop() {
  temp = SHT2x.GetTemperature();
  humi = SHT2x.GetHumidity();
  //lumi = (float)lightMeter.readLightLevel();
  Serial.println(temp);
  Serial.println(humi);
  //Serial.println(lumi);
  
  delay(2000);

}
