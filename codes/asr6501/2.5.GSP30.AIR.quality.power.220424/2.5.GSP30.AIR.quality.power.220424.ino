#include <Wire.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;


void setup() 
{
  Serial.begin(9600);
  pinMode(Vext, OUTPUT);
    digitalWrite(Vext,LOW); // start power before activating Wire
  Wire.begin(29,28);
  Serial.println("SGP30 test");
  while (!sgp.begin()){
    Serial.print(".");
    delay(500);
  }
  for(int i=0;i<4000;i++)
    {
    if (! sgp.IAQmeasure()) {
      Serial.println("Measurement failed");
      return;
      } 
    Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
    Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm"); 
    }
}

int counter = 0;

void loop() 
{
  digitalWrite(Vext,LOW); // start power before activating Wire
  Wire.begin(29,28);
  Serial.println("SGP30 test");
  while (!sgp.begin()){
    Serial.print(".");
    delay(500);
  }

  if (! sgp.IAQmeasure()) {
      Serial.println("Measurement failed");
      return;
      }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");

  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Wire.end();   // end Wire before disconnecting power
  digitalWrite(Vext,HIGH);
  delay(3000); 
}
