#include <Wire.h>
#include "Thinary_AHT10.h"

AHT10Class AHT10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(12,14);
  delay(1000);
  Serial.println("start"); Serial.println("start"); Serial.println("start");
  delay(1000);
  if(AHT10.begin(eAHT10Address_Low))
  if(AHT10.begin())
    Serial.println("Init AHT10 Sucess.");
  else
    Serial.println("Init AHT10 Failure.");

    delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(String("")+"Humidity(%RH):\t\t"+AHT10.GetHumidity()+"%");
  Serial.println(String("")+"Temperature(℃):\t"+AHT10.GetTemperature()+"℃");
  Serial.println(String("")+"Dewpoint(℃):\t\t"+AHT10.GetDewPoint()+"℃");
  Serial.println("start"); Serial.println("start"); Serial.println("start");
  delay(1500);
}
