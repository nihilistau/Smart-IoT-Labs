// esp32.HT.freeRTOS.11
static int taskCore0 = 0;
static int taskCore1 = 1;
 
void coreTask( void * pvParameters ){
    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    while(true){
        Serial.println(taskMessage);
        delay(1000);
    }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.print("Starting to create task on core ");
  Serial.println(taskCore0);
 
  xTaskCreatePinnedToCore(
                    coreTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore0);  /* Core where the task should run */
  Serial.println("Task created...");

  Serial.println(taskCore1);

  xTaskCreatePinnedToCore(
                    coreTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore1);  /* Core where the task should run */
  Serial.println("Task created...");
  
}
 
void loop() {
  Serial.println("Starting main loop...");
  while(true){
    delay(1000);
    }  // no delay – CPU is occupied 100%
}


