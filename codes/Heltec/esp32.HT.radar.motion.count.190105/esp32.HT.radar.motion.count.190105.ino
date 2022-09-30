#include <SPI.h>
#include <LoRa.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    870E6  //you can set band here directly,e.g. 868E6,915E6
int  sf=8;
int sb=125E3;       // set the signal bandwidth; 125KHz,[250KHz,500Kz]

int radPin = 17, val;
unsigned long start;
int count=0;

union 
  {
    uint8_t frame[28];
    struct 
      {
        uint8_t head[4];
        char mess[24];
      } data;
  } pack;

void taskSend( void * parameter)
{ 
  const TickType_t xDelay = 2000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  while(1)
  {
  if(count>0) 
    {
      Serial.println("message to send");
      LoRa.beginPacket();
      pack.data.head[0]=0xff;pack.data.head[1]=0x01;  // dest, source
      pack.data.head[2]=0x01;pack.data.head[3]=0x08;  // type, length
      strcpy(pack.data.mess,"alarm_d1");
      LoRa.write(pack.frame,pack.data.head[3]+4);
      LoRa.endPacket();
    }
  vTaskDelay(random(xDelay)+100);
  }
}

void setup() {
  // put your setup code here, to run once:
pinMode(17,INPUT);  
Serial.begin(9600);

  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  delay(100);
    xTaskCreate(
                    taskSend,          /* Task function. */
                    "taskSend",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);             // task handle

    Serial.println("Task Send created");
start=millis();
}

void loop() {
val = digitalRead(radPin);
if (val == LOW)
{
//  Serial.println("No motion");
//  Serial.println(millis());
  count=0;
}
else
{
//  Serial.println("Motion detected  ALARM");
//  Serial.println(millis());
  count++;
  Serial.println(count);
}
delay(100);

}
