#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  pinMode(Vext, OUTPUT);  
  Serial.begin(9600);
  Serial.println("Adafruit VL53L0X test");
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
}

void loop() {
  digitalWrite(Vext, LOW);
  delay(50);
  Wire.begin(29,28);
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    digitalWrite(Vext, HIGH);
    delay(1000);
    return;
  }
   
  //Serial.print("Reading a measurement... ");
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  Wire.end();
  digitalWrite(Vext, HIGH);
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); 
    Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println(" out of range ");
  }
  delay(100);
}
