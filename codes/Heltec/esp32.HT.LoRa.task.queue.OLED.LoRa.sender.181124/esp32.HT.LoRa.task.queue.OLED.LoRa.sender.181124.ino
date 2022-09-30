#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MLX90614.h>
#include "SHT21.h"


#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

SHT21 SHT21;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, 

QueueHandle_t oledqueue,LoRaqueue;
int queueSize=4;

long freq=868E6;
int sf=8, sb=125E3;
int devID=5;

union tspack
  {
    uint8_t frame[40];  // short data frame
    struct packet
      {
        uint8_t head[4];  // head[3] is length for sensors or text, text takes 1 on MSbit
        uint16_t num;
        uint16_t tout;
        union {
              float sensor[8];
              char text[32];
              } tsdata;
      } pack;
  } sdf,sbf,rdf,rbf;  // data frame and beacon frame



void taskLoRa( void * parameter)
{ 
  const TickType_t xDelay = 20000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  float sensors[4];
  int sdcount=0;

  while(true)
  {
  xQueueReceive(LoRaqueue,(float *)sensors,60000);
   sdf.pack.head[0]= 0xff;  // destination term - broadcast
   sdf.pack.head[1]= devID; // source term 3
   sdf.pack.head[2]= 0xf0;  // field mask - filed1,field2, field3
   sdf.pack.head[3]= 0x10;  // data size 16 bytes
   sdf.pack.num= (uint16_t) sdcount;
   sdf.pack.tout= (uint16_t) 0; // timeout 0 - here dev of rssi
   
   sdf.pack.tsdata.sensor[0]=sensors[0];
   sdf.pack.tsdata.sensor[1]=sensors[1];
   sdf.pack.tsdata.sensor[2]=sensors[2];
   sdf.pack.tsdata.sensor[3]=sensors[3];
  sdcount++;
  LoRa.beginPacket();
  LoRa.write(sdf.frame,40);
  LoRa.endPacket();
  Serial.println("LoRa packet sent");
  }

}

void taskOLED( void * parameter)
{ 
  const TickType_t xDelay = 10000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  //float temp,humi;
  float sensors[4];
  char dbuff[32]; int count=0;
  // Get temperature event and print its value.
  while(true)
  {
  xQueueReceive(oledqueue,(float *)sensors,10000);
  u8x8.clear();
  sprintf(dbuff,"ATemp:%2.2f*C",sensors[0]);
  u8x8.drawString(0,0,dbuff);
  sprintf(dbuff,"OTemp:%2.2f",sensors[1]);
  u8x8.drawString(0,1,dbuff);
  sprintf(dbuff,"Temp:%2.2f*C",sensors[2]);
  u8x8.drawString(0,2,dbuff);
  sprintf(dbuff,"Humi:%2.2f",sensors[3]);
  u8x8.drawString(0,3,dbuff);
  sprintf(dbuff,"Count:%4.4d",count); count++;
  u8x8.drawString(0,5,dbuff);
  vTaskDelay(xDelay);
  }
} 
  

void setup() { 
  char dbuff[32];
  Serial.begin(9600);
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();
  
  Wire.begin(21,22);   // initialize I2C bus - SDA, SCL
  delay(100);
  mlx.begin(); 
  delay(100);
  SHT21.begin();
  
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  delay(3000);
oledqueue = xQueueCreate(queueSize,4*sizeof(float));
LoRaqueue = xQueueCreate(queueSize,4*sizeof(float));

    xTaskCreate(
                    taskOLED,          /* Task function. */
                    "taskOLED",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL); 

    xTaskCreate(
                    taskLoRa,          /* Task function. */
                    "taskLoRa",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL); 
}

int count=0;
  
void loop() {      // task IDLE - priority 0
  float sensors[4];
  char dbuff[32];
  Serial.println("in the loop");
  xQueueReset(oledqueue); 
  xQueueReset(LoRaqueue); 
    sensors[0]=(float) mlx.readAmbientTempC();
    Serial.print("Ambient temp ");
    Serial.print(sensors[0]);
    Serial.println(" *C");
    delay(300);
    sensors[1]=(float)mlx.readObjectTempC();
    Serial.print("Object temp ");
    Serial.print(sensors[1]);
    Serial.println("%");
    delay(300);
    sensors[2]=(float)SHT21.getTemperature();
    Serial.print("Temp SHT21 ");
    Serial.print(sensors[2]);
    Serial.println("*C");
    delay(300);
    sensors[3]=(float)SHT21.getHumidity();
    Serial.print("Humi SHT21 ");
    Serial.print(sensors[3]);
    Serial.println("%");
    Serial.print("Count:");Serial.println(count); count++;
  xQueueSend(oledqueue,(float *)sensors,0);
  xQueueSend(LoRaqueue,(float *)sensors,0);
delay(30000);
}
