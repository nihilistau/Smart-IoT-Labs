
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_MLX90614.h>
#include "SHT21.h"

SHT21 SHT21;

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

float sensors[4];

void taskTS( void * parameter)
{ 
  const TickType_t xDelay = 20000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  float sensors[4];
  // Get temperature event and print its value.
  while(true)
  {
  //xSemaphoreTake(xSemSensTS,20000);   
  xQueueReceive(TSqueue,(float *)sensors,10000);
  ThingSpeak.setField(1, sensors[0]);
  ThingSpeak.setField(2, sensors[1]);
  ThingSpeak.setField(3, sensors[2]);
  ThingSpeak.setField(4, sensors[3]);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }
    ThingSpeak.writeFields(4, "4M9QG56R7VGG8ONT"); 
//  xSemaphoreGive(xSemSensTS);
//      delay(1000);
//  xSemaphoreGive(xSemSensTS);
    vTaskDelay(xDelay);
  }

}

void taskOLED( void * parameter)
{ 
  const TickType_t xDelay = 10000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  //float temp,humi;
  float sensors[4];
  char dbuff[32]; int count=0;
  // Get temperature event and print its value.
  while(true)
  {
  xQueueReceive(oledqueue,(float *)sensors,10000);
  u8x8.clear();
  sprintf(dbuff,"ATemp:%2.2f*C",sensors[0]);
  u8x8.drawString(0,0,dbuff);
  sprintf(dbuff,"OTemp:%2.2f",sensors[1]);
  u8x8.drawString(0,1,dbuff);
  sprintf(dbuff,"Temp:%2.2f*C",sensors[2]);
  u8x8.drawString(0,2,dbuff);
  sprintf(dbuff,"Humi:%2.2f",sensors[3]);
  u8x8.drawString(0,3,dbuff);
  sprintf(dbuff,"Count:%4.4d",count); count++;
  u8x8.drawString(0,5,dbuff);
  vTaskDelay(xDelay);
  }
} 
  

void taskMLX906( void * parameter)
{ 
  const TickType_t xDelay = 30000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  int count=0;
  float sensors[4];
  // Get temperature event and print its value.
  while(true)
  {
  //xSemaphoreTake(xSemSensTS,40000); 
    xQueueReset(oledqueue); 
    xQueueReset(TSqueue); 
    sensors[0]=(float) mlx.readAmbientTempC();
    Serial.print("Ambient temp ");
    Serial.print(sensors[0]);
    Serial.println(" *C");
    delay(300);
    sensors[1]=(float)mlx.readObjectTempC();
    Serial.print("Object temp ");
    Serial.print(sensors[1]);
    Serial.println("%");
    delay(300);
    sensors[2]=(float)SHT21.getTemperature();
    Serial.print("Temp SHT21 ");
    Serial.print(sensors[2]);
    Serial.println("*C");
    delay(300);
    sensors[3]=(float)SHT21.getHumidity();
    Serial.print("Humi SHT21 ");
    Serial.print(sensors[3]);
    Serial.println("%");
    Serial.print("Count:");Serial.println(count); count++;
//  xQueueReset(squeue);
//  xQueueSend(squeue,(float *)sensors,0);
  //xQueueReset(oledqueue);
  xQueueSend(oledqueue,(float *)sensors,0);
  //xQueueReset(TSqueue);
  xQueueSend(TSqueue,(float *)sensors,0);
  //xSemaphoreGive(xSemSensTS);
  vTaskDelay(xDelay);
  }
}

void setup() { 
  char dbuff[32];
  Serial.begin(9600);
  Wire.begin(21,22);   // SDA, SCL
  delay(100);
  mlx.begin(); 
  delay(100);
  SHT21.begin();
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


//squeue = xQueueCreate(queueSize,4*sizeof(float));
oledqueue = xQueueCreate(queueSize,4*sizeof(float));
TSqueue = xQueueCreate(queueSize,4*sizeof(float));


xSemSensTS = xSemaphoreCreateMutex();

  // Set delay between sensor readings based on sensor details.
     xTaskCreate(
                    taskMLX906,          /* Task function. */
                    "taskMLX906",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    3,                /* Priority of the task. */
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
                    2,                /* Priority of the task. */
                    NULL); 


}

void loop() {
  float sensors[4];
  char dbuff[32];
  Serial.println("in the loop");
//  xQueueReceive(squeue,(float *)sensors,10000);
//  Serial.print("Temp:");Serial.println(sensors[0]);
//  Serial.print("Humi:");Serial.println(sensors[1]);
delay(5000);
}
