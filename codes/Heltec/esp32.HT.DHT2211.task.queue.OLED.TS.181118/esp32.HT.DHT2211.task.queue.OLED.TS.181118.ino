
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
WiFiClient  client;
#define DHTPIN         17       // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11 
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, 

char ssid[] = "Livebox-08B0";          //  your network SSID (name) 
char pass[] = "G79ji6dtEptVTPWmZP";   // your network passw

uint32_t delayMS;

QueueHandle_t squeue,oledqueue,TSqueue;
int queueSize=4;

SemaphoreHandle_t xSemSensTS;

float sensors[2];

void taskTS( void * parameter)
{ 
  const TickType_t xDelay = 30000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  float sensors[2];
  // Get temperature event and print its value.
  while(true)
  {
  xSemaphoreTake(xSemSensTS,20000);   
  xQueueReceive(TSqueue,(float *)sensors,10000);
  ThingSpeak.setField(1, sensors[0]);
  ThingSpeak.setField(2, sensors[1]);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }
    ThingSpeak.writeFields(4, "4M9QG56R7VGG8ONT"); 
    xSemaphoreGive(xSemSensTS);
    vTaskDelay(xDelay);
  }

}

void taskOLED( void * parameter)
{ 
  const TickType_t xDelay = 3000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  //float temp,humi;
  float sensors[2];
  char dbuff[32];
  // Get temperature event and print its value.
  while(true)
  {
  xQueueReceive(oledqueue,(float *)sensors,10000);
  u8x8.clear();
  sprintf(dbuff,"Temp:%2.2f*C",sensors[0]);
  u8x8.drawString(0,0,dbuff);
  sprintf(dbuff,"Humi:%2.2f",sensors[1]);
  u8x8.drawString(0,2,dbuff);
  }
} 
  

void taskDHT22( void * parameter)
{ 
  const TickType_t xDelay = 10000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  //float temp,humi;
  float sensors[2];
  // Get temperature event and print its value.
  while(true)
  {
  xSemaphoreTake(xSemSensTS,20000); 
  sensors_event_t event;  
  delay(300);
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
    delay(300);
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
  xQueueReset(oledqueue);
  xQueueSend(oledqueue,(float *)sensors,0);
  xQueueReset(TSqueue);
  xQueueSend(TSqueue,(float *)sensors,0);
  delay(400);
  xSemaphoreGive(xSemSensTS);
  vTaskDelay(xDelay);
  }
}

void setup() { 
  char dbuff[32];
  Serial.begin(9600);

   WiFi.disconnect(true);
   delay(1000);
   WiFi.begin(ssid, pass);    
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("WiFi setup ok");
    ThingSpeak.begin(client);
    delay(1000);
  Serial.println("ThingSpeak begin");

  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.clear();
  u8x8.drawString(0,0,"IP address");
  sprintf(dbuff,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  u8x8.drawString(0,1,dbuff);
  delay(100);
  u8x8.drawString(0,3,"Start TS");
  delay(3000);
  // Initialize device.
  dht.begin();
//  Serial.println("DHTxx Unified Sensor Example");
//  // Print temperature sensor details.
//  sensor_t sensor;
//  dht.temperature().getSensor(&sensor);
//  Serial.println("------------------------------------");
//  Serial.println("Temperature");
//  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
//  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
//  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
//  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
//  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
//  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
//  Serial.println("------------------------------------");
//  // Print humidity sensor details.
//  dht.humidity().getSensor(&sensor);
//  Serial.println("------------------------------------");
//  Serial.println("Humidity");
//  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
//  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
//  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
//  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
//  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
//  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
//  Serial.println("------------------------------------");


squeue = xQueueCreate(queueSize,2*sizeof(float));
oledqueue = xQueueCreate(queueSize,2*sizeof(float));
TSqueue = xQueueCreate(queueSize,2*sizeof(float));


xSemSensTS = xSemaphoreCreateMutex();

  // Set delay between sensor readings based on sensor details.
     xTaskCreate(
                    taskDHT22,          /* Task function. */
                    "taskDHT22",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */

    xTaskCreate(
                    taskOLED,          /* Task function. */
                    "taskOLED",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL); 

    xTaskCreate(
                    taskTS,          /* Task function. */
                    "taskTS",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
                    NULL); 


}

void loop() {
  float sensors[2];
  char dbuff[32];
  Serial.println("in the loop");
  xQueueReceive(squeue,(float *)sensors,10000);
  Serial.print("Temp:");Serial.println(sensors[0]);
  Serial.print("Humi:");Serial.println(sensors[1]);

}
