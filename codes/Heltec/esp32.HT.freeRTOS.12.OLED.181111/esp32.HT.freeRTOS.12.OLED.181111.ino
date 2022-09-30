// esp32.HT.freeRTOS.12.Oled.Wifi.TS
#include <WiFi.h>
#include "ThingSpeak.h"
#include <U8x8lib.h>
#include <Wire.h>
#include <SHT21.h>
SHT21 sht;   

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

char ssid[] = "Livebox-08B0";          //  your network SSID (name) 
char pass[] = "G79ji6dtEptVTPWmZP";   // your network passw
WiFiClient  client;

QueueHandle_t queue;

struct {
  float t;
  float h;
} ssens,rsens;  // send sensors, receive sensors

int queueSize = 128;
 
void sensorTask( void * pvParameters ){
    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    while(true){
        Serial.println(taskMessage);
        ssens.t=sht.getTemperature(); 
        ssens.h=sht.getHumidity();
        xQueueReset(queue); // keeps only last element
        xQueueSend(queue, &ssens, portMAX_DELAY);
        delay(30000);  // should be longer than the sending cycle
    }
}

void ThingSpeakTask( void * pvParameters ){
    char dbuff[32];
    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    while(true){
      xQueueReceive(queue, &rsens, portMAX_DELAY);
      Serial.print("temp:");Serial.println(rsens.t);
      Serial.print("humi:");Serial.println(rsens.h);
      u8x8.clear();
      sprintf(dbuff,"Temp:%4.2f",rsens.t);
      u8x8.drawString(0,1,dbuff);
      sprintf(dbuff,"Humi:%4.2f",rsens.h);
      u8x8.drawString(0,2,dbuff);
        ThingSpeak.setField(1, rsens.t);
        ThingSpeak.setField(2, rsens.h);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
          }
        ThingSpeak.writeFields(1, "HEU64K3PGNWG36C4");  // channel 1 on ThingSpeak.fr
        delay(15000);
       }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  WiFi.disconnect(true);delay(1000);
  WiFi.begin(ssid, pass);
  Serial.println(WiFi.getMode());delay(1000);     
    
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("WiFi setup ok");
  delay(1000);
  Serial.println(WiFi.status());
  ThingSpeak.begin(client);
    delay(1000);
  Serial.println("ThingSpeak begin");;
  delay(1000);
    queue = xQueueCreate(queueSize,8);  // 2 floats
  Serial.print("Starting to create task on core 0");

  xTaskCreatePinnedToCore(
                    sensorTask,   /* Function to implement the task */
                    "sensorTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    0);  /* Core where the task should run */
  Serial.println("Task created...");
  
 Serial.print("Starting to create task on core 1");

  xTaskCreatePinnedToCore(
                    ThingSpeakTask,   /* Function to implement the task */
                    "ThingSpeaTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    1);  /* Core where the task should run */
  Serial.println("Task created...");
}
 
void loop() {

  Serial.print("loop(): executing on core ");
  Serial.println(xPortGetCoreID());
delay(5000);
}


