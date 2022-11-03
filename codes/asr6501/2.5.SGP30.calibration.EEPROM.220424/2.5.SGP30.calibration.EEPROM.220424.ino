    #include <Wire.h>
    #include "Adafruit_SGP30.h"
    #include <EEPROM.h>

    Adafruit_SGP30 sgp;

    /* return absolute humidity [mg/m^3] with approximation formula
      @param temperature [°C]
      @param humidity [%RH]
    */

    const int EEpromWrite = 2; // intervall in which to write new baselines into EEPROM in hours
    unsigned long previousMillis = 0; // Millis at which the intervall started
    uint16_t TVOC_base;
    uint16_t eCO2_base;

    uint32_t getAbsoluteHumidity(float temperature, float humidity) {
      // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
      const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
      const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
      return absoluteHumidityScaled;
    }

    void setup() {
      Serial.begin(9600);
      Serial.println("SGP30 test");

      if (! sgp.begin()) {
        Serial.println("Sensor not found :(");
        while (1);
      }
      Serial.print("Found SGP30 serial #");
      Serial.print(sgp.serialnumber[0], HEX);
      Serial.print(sgp.serialnumber[1], HEX);
      Serial.println(sgp.serialnumber[2], HEX);

      // This will get the previous baseline from EEPROM
      // But it will only write it to the sensor if there really has been one before
      // On first run this will be empty. This will put the sensor into a special 12 hour baseline
      // finding mode. Also the sensor needs to "burn in". Optimal would be if, on first startup,
      // you let the sensor run for 48 hours. It will store a new baseline every X hours (as set bellow)
      // Make sure the sensor is exposed to outside air multiple times, during the first 48 hours, but especially
      // within the last three hours before turning it off the first time.
      // On next start up the last baseline will be in EEPROM and will be given to the sensor. From then on
      // it will always get a new baseline itself and the code will return it to the sensor in case of a power down.
      // The sensor should be exposed to outside air once a week to make sure the baseline is of good quality.
     
      EEPROM.get(1, TVOC_base);
      EEPROM.get(10, eCO2_base);
      if (eCO2_base != 0) sgp.setIAQBaseline(TVOC_base, eCO2_base);
      Serial.print("****Baseline values in EEPROM: eCO2: 0x"); Serial.print(eCO2_base, HEX);
      Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);

    }

    int counter = 0;
    void loop() {
      // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
      //float temperature = 22.1; // [°C]
      //float humidity = 45.2; // [%RH]
      //sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

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
      Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
      Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");

      delay(1000);

      counter++;
      if (counter == 30) {
        counter = 0;

        if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
          Serial.println("Failed to get baseline readings");
          return;
        }
        Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
        Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
      }


      // Prepare the EEPROMWrite intervall
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= EEpromWrite * 3600000) {
        previousMillis = currentMillis; // reset the loop
        sgp.getIAQBaseline(&eCO2_base, &TVOC_base); // get the new baseline
        EEPROM.put(1, TVOC_base); // Write new baselines into EEPROM
        EEPROM.put(10, eCO2_base);
      }


    }
