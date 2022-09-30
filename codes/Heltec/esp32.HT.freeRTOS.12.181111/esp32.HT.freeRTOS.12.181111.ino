// esp32.HT.freeRTOS.12
#include <Wire.h>
#include <SHT21.h>
SHT21 sht;   
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
  Serial.println("Starting main loop...");
  while (true){
    Serial.println("main on core 1");
    xQueueReceive(queue, &t, portMAX_DELAY);
    xQueueReceive(queue, &h, portMAX_DELAY);
    Serial.print("temp:");Serial.println(t);
    Serial.print("humi:");Serial.println(h);
    delay(1000);
    }
}


