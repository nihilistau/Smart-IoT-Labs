
#include <WiFi.h>
#include "esp_wpa2.h"
#include "ThingSpeak.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

#define DHTPIN         17       // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)

WiFiClient  client;
char ssid[] = "Livebox-08B0";          //  your network SSID (name) 
char pass[] = "G79ji6dtEptVTPWmZP";   // your network passw

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

QueueHandle_t squeue;
int queueSize=4;

float sensors[2];

void taskOLED( void * parameter)
{ 
  const TickType_t xDelay = 3000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  char dbuff[32];
  float sensors[2];
  while(true)
    {
    delay(1000);
    xQueuePeek(squeue,(float *)sensors,10000);
    u8x8.clear();
    u8x8.drawString(0,0,"Sensors T/H");
    delay(1000);
    sprintf(dbuff,"Temp:%2.2f*C",sensors[0]);
    u8x8.drawString(0,2,dbuff);
    delay(1000);
    sprintf(dbuff,"Humi:%2.2f%",sensors[1]);
    u8x8.drawString(0,4,dbuff);
    vTaskDelay(xDelay);
    }
 }


void taskDHT22( void * parameter)
{ 
  const TickType_t xDelay = 3000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  //float temp,humi;
  float sensors[2];
  // Get temperature event and print its value.
  while(true)
  {
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    sensors[0]=(float)event.temperature;
    Serial.print(sensors[0]);
    Serial.println(" *C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    sensors[1]=(float)event.relative_humidity;
    Serial.print(sensors[1]);
    Serial.println("%");
  }
  xQueueSend(squeue,(float *)sensors,0);
  vTaskDelay(xDelay);
  }
}

void setup() { 
  Serial.begin(9600); 
//  u8x8.begin();  // initialize OLED
//  u8x8.setFont(u8x8_font_chroma48medium8_r);
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");

  // Set delay between sensor readings based on sensor details.
  xTaskCreate(
                    taskDHT22,          /* Task function. */
                    "taskDHT22",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    0,                /* Priority of the task. */
                    NULL);  
                    
// xTaskCreate(
//                    taskOLED,          /* Task function. */
//                    "taskOLED",        /* String with name of task. */
//                    10000,            /* Stack size in words. */
//                    NULL,             /* Parameter passed as input of the task */
//                    0,                /* Priority of the task. */
//                    NULL);            /* Task handle. *//* Task handle. */

squeue = xQueueCreate(queueSize,2*sizeof(float));

}

void loop() {
  float sensors[2];
  Serial.println("in the loop");
  xQueueReceive(squeue,(float *)sensors,10000);
  Serial.print("Temp:");Serial.println(sensors[0]);
  Serial.print("Humi:");Serial.println(sensors[1]);
 
}
