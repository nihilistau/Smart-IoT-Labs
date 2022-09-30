
#include "Adafruit_CCS811.h"

Adafruit_CCS811 ccs;

void setup()
{
  Serial.begin(9600);
  Wire.begin(12,14);
  delay(200);
  Serial.println("CCS811 test");
  
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
    //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
}

float etemp=20.0;
int co2,tvoc; char dbuff[24];

void loop() {
  if(ccs.available()){
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
      Serial.print("CO2: ");
      co2=ccs.geteCO2();
      Serial.print(co2);

      tvoc=ccs.getTVOC();
 
      Serial.print("ppm, TVOC: ");
      Serial.print(tvoc);
      Serial.print("ppb   Temp:");
      etemp=0.02*(temp-2.00)+ 0.98*etemp;
      Serial.println(temp);
      Serial.print("Est temp:");Serial.println(etemp);
   
    }
    else{
      Serial.println("ERROR!");
      delay(5000);
    }
  }

}
