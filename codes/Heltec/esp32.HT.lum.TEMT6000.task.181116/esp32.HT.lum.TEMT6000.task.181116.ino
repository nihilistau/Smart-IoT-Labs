#define LEDPIN 25         //LED brightness (PWM) writing
#define LIGHTSENSORPIN 36 //Ambient light sensor reading 

void taskTEMT6000( void * parameter)
{ 
  const TickType_t xDelay = 1000 / portTICK_PERIOD_MS; 
  float reading,square_ratio;
  float voltage;
  
    while(true)
    {
    reading = analogRead(LIGHTSENSORPIN); //Read light level
    voltage = reading*(3.3/4095.0);
    Serial.print("Reading:");Serial.println(reading);
    Serial.print("Voltage:");Serial.println(voltage);


    if(reading>100) digitalWrite(LEDPIN,HIGH);  //Adjust LED brightness relatively
    else  digitalWrite(LEDPIN,LOW);
                    //Display reading in serial monitor
    vTaskDelay(xDelay);
    }
}

void setup() {
  pinMode(LIGHTSENSORPIN,  INPUT);  
  pinMode(LEDPIN, OUTPUT);  
  Serial.begin(9600);
   xTaskCreate(
                    taskTEMT6000,          /* Task function. */
                    "TaskTEMT6000",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
  
}

void loop() {
Serial.println("in IDLE task");
  delay(3000);
}
