#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

void setup() {
  Serial.begin(9600);
  while(!Serial); Serial.println();
  Wire.begin(7,8);
  delay(2000);
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
  Serial.println("BH1750 One-Time Test");
}

void loop() {
  while (!lightMeter.measurementReady(true)) {   yield();  }
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  delay(5000);
}
