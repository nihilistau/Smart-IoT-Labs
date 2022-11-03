#include "Arduino.h"
#include <Wire.h>
#include "VL53L1X.h"
VL53L1X sensor;

void setup(void)
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(500);
  Serial.begin(9600);
  Serial.println();Serial.println();delay(100); 
  Wire.begin(29,28);
  Wire.setClock(400000); // use 400 kHz I2C
  sensor.setTimeout(500);
  Serial.println("before init");
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);
}

void loop()
{
  Serial.print(sensor.read());
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  Serial.println();
}
