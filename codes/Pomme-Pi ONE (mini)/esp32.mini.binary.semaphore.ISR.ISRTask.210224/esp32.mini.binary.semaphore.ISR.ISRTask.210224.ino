byte ledPin = 22;
/* pin that is attached to interrupt */
byte interruptPin = 2;
/* hold the state of LED when toggling */
volatile byte state = LOW;
SemaphoreHandle_t xBinarySemaphore;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  /* set the interrupt pin as input pullup*/
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISRcallback, CHANGE);
  /* initialize binary semaphore */
  xBinarySemaphore = xSemaphoreCreateBinary();
  /* this task will process the interrupt event 
  which is forwarded by interrupt callback function */
  xTaskCreate(
    ISRprocessing,           /* Task function. */
    "ISRprocessing",         /* name of task. */
    1000,                    /* Stack size of task */
    NULL,                    /* parameter of the task */
    4,                       /* priority of the task */
    NULL);  
}

void loop() {
}
/* interrupt function callback */
void ISRcallback() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /* un-block the interrupt processing task now */
  xSemaphoreGiveFromISR(xBinarySemaphore,&xHigherPriorityTaskWoken );
}

/* this function will be invoked when additionalTask was created */
void ISRprocessing( void * parameter )
{
  Serial.println((char *)parameter);
  /* loop forever */
  for(;;){
    /* task move to Block state to wait for interrupt event */
    xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
    Serial.println("ISRprocessing is running");
    /* toggle the LED now */
    state = !state;
    digitalWrite(ledPin, state);
  }
  vTaskDelete( NULL );
}
