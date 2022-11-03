#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter;

void setup()
{
  Serial.begin(9600);
  pinMode(Vext,OUTPUT);
//  digitalWrite(Vext,LOW); //3V3 voltage output activated - Vext ON
//  Wire.begin(29,28);
//  lightMeter.begin();
//  Serial.println(F("BH1750 Test begin"));
}

float lux;

void loop() 
{
  digitalWrite(Vext,LOW); // start power before activating Wire
  Wire.begin(29,28);
  delay(200);
  lightMeter.begin();
  delay(200);
  lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux");
  Wire.end();  // end Wire before disconnecting power
  digitalWrite(Vext,HIGH);
  delay(3000);
}
