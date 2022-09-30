/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

void setup() {
  Serial.begin(9600);
  Wire.begin(12,14);
  Serial.println();
  Serial.println(F("BME680 test"));
  delay(1000);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

float temper, pres, humi, gas, comp_gas;

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  temper=bme.temperature;
  Serial.print(temper);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  pres=bme.pressure;
  Serial.print(pres/100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  humi=bme.humidity;
  Serial.print(humi);
  Serial.println(" %");

  Serial.print("Gas = ");
  gas = bme.gas_resistance;
  Serial.print(gas/1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
  comp_gas=log(gas) +0.04*humi;
  Serial.print("Air quality = ");Serial.println(comp_gas);
  Serial.println();
  delay(2000);
}
