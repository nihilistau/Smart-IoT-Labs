// esp32.HT.freeRTOS.12
#include <WiFi.h>
#include "ThingSpeak.h"
#include <U8x8lib.h>
#include <Wire.h>
#include <SHT21.h>
SHT21 sht;   

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

char ssid[] = "YotaPhoneAP";          //  your network SSID (name) 
char pass[] = "tonbridge";   // your network passw
WiFiClient  client;

QueueHandle_t queue;
int queueSize = 128;
static int taskCore = 1;  // or 0
 
void sensorTask( void * pvParameters ){
    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    float t,h;
    while(true){
        Serial.println(taskMessage);
        t=sht.getTemperature(); 
        h=sht.getHumidity();
         xQueueSend(queue, &t, portMAX_DELAY);
         xQueueSend(queue, &h, portMAX_DELAY);
        delay(1000);
    }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  delay(1000);
    queue = xQueueCreate( queueSize, sizeof( int ) );
  Serial.print("Starting to create task on core ");
  Serial.println(taskCore);
  xTaskCreatePinnedToCore(
                    sensorTask,   /* Function to implement the task */
                    "sensorTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}
 
void loop() {
  float t,h;
  char dbuff[32];
  Serial.println("Starting main loop...");
  while (true){
    Serial.println("main on core 1");
    xQueueReceive(queue, &t, portMAX_DELAY);
    xQueueReceive(queue, &h, portMAX_DELAY);
    Serial.print("temp:");Serial.println(t);
    Serial.print("humi:");Serial.println(h);
    u8x8.clear();
    sprintf(dbuff,"Temp:%4.2f",t);
    u8x8.drawString(0,1,dbuff);
    sprintf(dbuff,"Humi:%4.2f",h);
    u8x8.drawString(0,2,dbuff);
    
    delay(1000);
    }
}


