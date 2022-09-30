#include <Wire.h>
#include "Adafruit_HTU21DF.h"
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
QueueHandle_t queue;
int queueSize = 128;
static int taskCore = 0;
 
void sensorTask( void * pvParameters ){

    float t,h;
    while(true){
        Serial.printf("Sensor task running on core: %d \n",xPortGetCoreID());
        t=htu.readTemperature();
        h=htu.readHumidity();
         xQueueSend(queue, &t, portMAX_DELAY);
         xQueueSend(queue, &h, portMAX_DELAY);
        delay(1000);
    }
}

void setup() {
  Serial.begin(9600);
  Wire.begin(12,14);
  delay(1000);
    queue = xQueueCreate( queueSize, sizeof( int ) );
   Serial.println("HTU21D-F test");
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
    }
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
  Serial.println("Starting main loop...");
  while (true){
    Serial.printf("Main task on core: %d\n",xPortGetCoreID());
    xQueueReceive(queue, &t, portMAX_DELAY);
    xQueueReceive(queue, &h, portMAX_DELAY);
    Serial.print("temp:");Serial.println(t);
    Serial.print("humi:");Serial.println(h);
    delay(1000);
    }
}
