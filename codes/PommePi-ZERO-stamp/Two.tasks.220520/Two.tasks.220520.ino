
#define RUNNING_CORE 0  // only one core

// define two tasks 
void TaskHello( void *pvParameters );
void TaskBonjour( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() 
{
  Serial.begin(9600);
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
      TaskHello,
       "TaskHello",   // A name just for humans
       1024,  // stack size can be checked & adjusted by reading the Stack Highwater
       NULL,
       2,  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
       NULL, 
      RUNNING_CORE);

  xTaskCreatePinnedToCore(
      TaskBonjour,
       "TaskBonjour",
       1024,  // Stack size
       NULL,
       1,  // Priority
       NULL, 
       RUNNING_CORE);

  // Now the task scheduler, which takes over control of scheduling individual tasks, 
  // is automatically started.
}

void loop()
{
  // Things are done in the background tasks.
  Serial.println("in the loop task ");
  delay(2000);
}

void TaskHello(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    Serial.println("Hello 1");
    vTaskDelay(1000);   
    Serial.println("Hello 2");
    vTaskDelay(1000);   
  }
}

void TaskBonjour(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  for (;;) // A Task shall never return or exit.
  {
    Serial.println("Bonjour 1");
    vTaskDelay(1000);   
    Serial.println("Bonjour 2");
    vTaskDelay(1000);   
  }
}
