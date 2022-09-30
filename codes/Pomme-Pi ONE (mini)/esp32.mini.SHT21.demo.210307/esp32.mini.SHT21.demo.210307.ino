/****************************************************************
*  SHT21_Demo
*
*  An example sketch that reads the sensor and prints the
*  relative humidity to the serial port
* 
***************************************************************/

#include <Wire.h>
#include "SHT21.h"

uint16_t timeout=10;

SHT21 SHT21;

void setup()
{
  Wire.begin(12,14);
  SHT21.begin();
  Serial.begin(9600);
    Serial.print("Humidity(%RH): ");
  Serial.print(SHT21.getHumidity());
  Serial.print("     Temperature(C): ");
  Serial.println(SHT21.getTemperature());
esp_sleep_enable_timer_wakeup(1000*1000*timeout);  // in micro-seconds - timeout in seconds

esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);



  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  Serial.println("Going to sleep now");
  Serial.flush(); delay(60); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop()
{
//  Serial.print("Humidity(%RH): ");
//  Serial.print(SHT21.getHumidity());
//  Serial.print("     Temperature(C): ");
//  Serial.println(SHT21.getTemperature());
//  
//  delay(1000);
}
