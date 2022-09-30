
#include <SPI.h>             
#include <LoRa.h>
#include <U8x8lib.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    868E6  //you can set band here directly,e.g. 868E6,915E6
#define sf 8
#define sb 125E3       // set the signal bandwidth; 125KHz,[250KHz,500Kz]

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

int i=0,rssi;

QueueHandle_t bqueue, dqueue;  // queues for beacons and data frames

union tspack
  {
    uint8_t frame[128];
    struct packet
      {
        uint8_t head[4];
        uint16_t num;
        uint16_t tout;
        float sensor[4];
      } pack;
  } sdf,sbf,rdf,rbf;  // data frame and beacon frame

  int sdcount=0, sbcount=0, rdcount=0, rbcount=0;
  int sr=1;

void dispData(int sr)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SPacket:%4.4d",sdcount);
  else sprintf(dbuf,"RPacket:%4.4d",rdcount);
  u8x8.drawString(0, 1,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Light:%5.5f",sdf.pack.sensor[1]);
  else sprintf(dbuf,"Light:%5.5f",rdf.pack.sensor[1]);
  u8x8.drawString(0, 2,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Temp:%2.2f",sdf.pack.sensor[2]);
  else sprintf(dbuf,"Temp:%2.2f",rdf.pack.sensor[2]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Humi:%2.2f",sdf.pack.sensor[3]);
  else sprintf(dbuf,"Humi:%2.2f",rdf.pack.sensor[3]);
  u8x8.drawString(0, 4,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"RSSI:%4.4d",rssi);
  else sprintf(dbuf,"                ");
  u8x8.drawString(0, 6,dbuf);
}

void dispPara()
{
  char dbuf[16];
  Serial.println("LoRa LP Recv");
  u8x8.drawString(0, 1, "LoRa LP Recv");
  sprintf(dbuf,"Freq:%3.2f MHz",(float)freq/1000000.0);
  u8x8.drawString(0, 2, dbuf);
  LoRa.setSpreadingFactor(sf);
  sprintf(dbuf,"SF:%d",sf);
  u8x8.drawString(0, 3, dbuf);
  LoRa.setSignalBandwidth(sb);
  sprintf(dbuf,"SB:%3.2f KHz",(float)sb/1000.0);
  u8x8.drawString(0, 4, dbuf);
  delay(6000);
  u8x8.clear();
}

void dispBeacon()
{
  char dbuf[16];
  u8x8.clear();
  Serial.println("ACK Beacon");
  u8x8.drawString(0, 1, "ACK Beacon");
  sprintf(dbuf,"Beacon:%d",(int)sbf.pack.num);
  u8x8.drawString(0, 2, dbuf);
  sprintf(dbuf,"Tout:%d",(int)sbf.pack.tout);
  u8x8.drawString(0, 3, dbuf);

}


void onReceive(int packetSize) 
{
uint8_t rdbuff[24], rbbuff[8];  
  if (packetSize == 0) return;   // if there's no packet, return
  i=0;
  if (packetSize==24)
    {
    while (LoRa.available()) {
      rdbuff[i]=LoRa.read();i++;
      }
      //dispData(0); 
      //Serial.println("recv data");
      rssi=LoRa.packetRssi();
      xQueueSend(dqueue, rdbuff, portMAX_DELAY); 
    }
  if (packetSize==8)
    {
    while (LoRa.available()) {
      rbbuff[i]=LoRa.read();i++;
      }
       //dispBeacon();
      rssi=LoRa.packetRssi();
      xQueueSend(bqueue, rbbuff, portMAX_DELAY);
    }  
  delay(200);
}


int timeout=30, nextdata=17;
int ticks=1000; // 1 tick is 10 ms

void ReadLoRaDataTask( void * pvParameters )
{  
while(1)
  {     
    xQueueReceive(dqueue, rdf.frame, portMAX_DELAY); // portMAX_DELAY in millisecondes
    dispData(0);
    delay(200); 
    LoRa.receive(); 
    }  
} 

void setup() {
  Serial.begin(9600);                   // initialize serial
  pinMode(26, INPUT);  // recv interrupt
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.println("LoRa recv LP");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  dispPara();
  Serial.println("LoRa init succeeded.");

  dqueue = xQueueCreate(4, 24); // queue for 4 LoRaTS data frames
  bqueue = xQueueCreate(4, 8); // queue for 4 LoRaTS beacon frames
  
     xTaskCreate(
                    ReadLoRaDataTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL     );  /* Task handle */
 
  Serial.println("ReadLoRaDataTask created...");
  LoRa.onReceive(onReceive);
  LoRa.receive(); 
  delay(3000);
}



void loop() 
{
delay(15000);
}




