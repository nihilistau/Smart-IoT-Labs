
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

WiFiClient  client;


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
  

void taskMLX906( void * parameter)
{ 
  const TickType_t xDelay = 10000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  //float temp,humi;
  float sensors[2];
  // Get temperature event and print its value.
  while(true)
  {
  xSemaphoreTake(xSemSensTS,20000);   
  delay(300);
    sensors[0]=(float) mlx.readAmbientTempC();
    Serial.print("Ambient temp ");
    Serial.print(sensors[0]);
    Serial.println(" *C");
    delay(300);
    sensors[1]=(float)mlx.readObjectTempC();
    Serial.print("Object temp ");

    Serial.print(sensors[1]);
    Serial.println("%");
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
  Wire.begin(21,22);   // SDA, SCL
  delay(100);
  mlx.begin(); 
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


squeue = xQueueCreate(queueSize,2*sizeof(float));
oledqueue = xQueueCreate(queueSize,2*sizeof(float));
TSqueue = xQueueCreate(queueSize,2*sizeof(float));


xSemSensTS = xSemaphoreCreateMutex();

  // Set delay between sensor readings based on sensor details.
     xTaskCreate(
                    taskMLX906,          /* Task function. */
                    "taskMLX906",        /* String with name of task. */
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
