#include <SPI.h>
#include <LoRa.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    434E6  //you can set band here directly,e.g. 868E6,915E6
int  sf=8;
int sb=125E3;       // set the signal bandwidth; 125KHz,[250KHz,500Kz]
uint8_t sw=0xF3;

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock,

int radPin = 17, val;
unsigned long start;
int count=0;
char dbuff[32];
int mc=0;

union 
  {
    uint8_t frame[8];
    struct 
      {
        uint8_t head[4];
        uint32_t numb;
      } data;
  } pack;

void taskSend( void * parameter)
{ 
  const TickType_t xDelay = 2000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  while(1)
  {
  if(count>0) 
    {
      LoRa.beginPacket();
      pack.data.head[0]=0xff;pack.data.head[1]=0x01;  // dest, source
      pack.data.head[2]=0x01;pack.data.head[3]=0x04;  // type, length
      pack.data.numb= (uint32_t)mc;
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
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
Serial.println();
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(434E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa OK!");
  u8x8.drawString(0,0,"LoRa OK");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  LoRa.setSyncWord(sw);


  
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
LoRa.setFrequency(868E6);
delay(20);
val = digitalRead(radPin);
delay(20);
LoRa.setFrequency(434E6);
delay(20);
LoRa.setFrequency(434E6);
if (val == LOW)
{
  count=0;
}
else
{
//  Serial.println("Motion detected  ALARM");
//  Serial.println(millis());
  u8x8.drawString(0,1,"Mouvement !");
  mc++;count++;
  sprintf(dbuff,"Num:%6.6d",mc);
  u8x8.drawString(0,2,dbuff);
  //Serial.println(count);

}
//LoRa.setFrequency(868E6);
delay(20);

}
