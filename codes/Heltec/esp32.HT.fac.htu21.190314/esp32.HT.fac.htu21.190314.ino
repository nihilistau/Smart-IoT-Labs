#include <Wire.h> // driver I2C
#include "SparkFunHTU21D.h"

 HTU21D myTempHumi;
 void setup()
 {
 Serial.begin(9600);
 //Wire.begin(21,22);  // carte d'extension I2C2
 //Wire.begin(22,21);  // carte d'extension I2C1
 Serial.println("HTU21D Example!");
 myTempHumi.begin();
 delay(1000);
 }
 
void loop()
{
float humd = myTempHumi.readHumidity();
float temp = myTempHumi.readTemperature();
delay(1000);
Serial.print(" Temperature:");
Serial.print(temp, 1);
Serial.print("C");
Serial.print(" Humidity:");
Serial.print(humd, 1);
Serial.print("%");
Serial.println();
delay(1000);
}


