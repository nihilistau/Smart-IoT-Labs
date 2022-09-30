
#include <SPI.h>
#include <LoRa.h>

#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include <BH1750.h>

BH1750 lightMeter;
// sensitivity table
int senst[5][7] = { // 6     7    8    9   10   11   12   - spreading factor
                    {-125,-129,-133,-136,-138,-142,-143}, //  31250 Hz
                    {-121,-126,-129,-132,-135,-138,-140}, //  62500 Hz
                    {-118,-123,-126,-129,-132,-135,-137}, // 125000 Hz
                    {-115,-120,-123,-126,-129,-132,-134}, // 250000 Hz
                    {-112,-117,-120,-123,-126,-129,-131}, // 500000 Hz
};


#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset
#include "introd.h"  // configuration function

int rssi;
QueueHandle_t bqueue, dqueue, squeue;  // queues for beacons and data frames
int sdcount=0, rdcount=0;

union tspack
  {
    uint8_t frame[128];
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

union
 {
  uint8_t frame[16];
  float senst[4]; 
 }
 wsensb,rsensb;  // sensor block

void dispLabel()
{
  u8x8.clear();
  u8x8.draw2x2String(0,0,"Polytech");
  delay(1000);
  u8x8.draw2x2String(3,2,"IMT");
  delay(1000);
  u8x8.draw2x2String(4,4,"LoRa");
  delay(1000);
  u8x8.draw2x2String(5,6,"link");
  delay(3000);
  u8x8.clear();
}


void dispDatarate(int dr,int sf, int sb)
{
  char dbuf[16];int sbind=0;int sensitivity=0;
  u8x8.clear();
  sprintf(dbuf,"DR=%5.5d bps",dr);
  u8x8.drawString(0,1,dbuf);
  delay(1000);
  u8x8.drawString(0,2,"CR=4/8");
  delay(1000);
  u8x8.drawString(0,3,"CRC check set");
  switch (sb)
    {
    case 31250: sbind=0;break;
    case 62500: sbind=1;break;
    case 125000: sbind=2;break;
    case 250000: sbind=3;break;
    case 500000: sbind=4;break;
    default: sbind=0;break;
    }

    Serial.println(sf-6);Serial.println(sbind);
  delay(1000);  
  sensitivity=senst[sbind][sf-6];
  sprintf(dbuf,"Sens=%3.3d dBm",sensitivity);
  u8x8.drawString(0,4,dbuf);
  delay(1000);  
  u8x8.drawString(0,6,"Packet size 24B");
  delay(3000);
  u8x8.clear();
}

int bitrate(int psf, int psb)  // by default cr=4
{
  int res;
  res= (int) ((float)psf*(float)psb/(2.0*pow(2,psf)));
  return res;
}

void dispData(int sr)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SPacket:%5.5d",sdcount);
  else sprintf(dbuf,"RPacket:%5.5d",rdcount);
  u8x8.drawString(0, 1,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Temp:%2.2f",sdf.pack.tsdata.sensor[0]);
  else sprintf(dbuf,"Temp:%2.2f",rdf.pack.tsdata.sensor[0]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Humi:%2.2f",sdf.pack.tsdata.sensor[1]);
  else sprintf(dbuf,"Humi:%2.2f",rdf.pack.tsdata.sensor[1]);
  u8x8.drawString(0, 4,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Lumi:%5.5d",(int)sdf.pack.tsdata.sensor[2]);
  else sprintf(dbuf,"Lumi:%5.5d",(int)rdf.pack.tsdata.sensor[2]);
  u8x8.drawString(0, 5,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"Srssi:%3.3d",(int)sdf.pack.tsdata.sensor[3]);
  u8x8.drawString(0, 6,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"Rrssi:%3.3d",rssi);
  else sprintf(dbuf,"                ");
  u8x8.drawString(0, 7,dbuf);
  if(!sr) delay(3000);
}

void SensorTask()
{
   while(1)
     {
      wsensb.senst[0]= SHT2x.GetTemperature();
      delay(300); 
      wsensb.senst[1]= SHT2x.GetHumidity();
      delay(300); 
      wsensb.senst[2]= (float)lightMeter.readLightLevel();
      wsensb.senst[0]= 0.0;
      xQueueReset(squeue); // to keep only the last element
      xQueueSend(squeue, wsensb.frame, portMAX_DELAY);
      delay(3000); 
     }
}

void LoRaSendTask( void * pvParameters )
{
int i=0;
//TickType_t xLastWakeTime;
//const TickType_t xFrequency = 19910; //17700 -1 tick 100ms
//xLastWakeTime = xTaskGetTickCount();
while(1)
  {  
  //vTaskDelayUntil( &xLastWakeTime, xFrequency );
  LoRa.beginPacket();
  sdf.pack.head[0]= 0x01;  // destination term 1
  sdf.pack.head[1]= 0x02;  // source term 2
  sdf.pack.head[2]= 0xe0;  // field mask - filed1,field2, field3
  sdf.pack.head[3]= 0x10;  // data size 16 bytes
  sdf.pack.num= (uint16_t) sdcount;
  sdf.pack.num= (uint16_t) 0; // timeout 0

  //xQueueReceive(squeue, rsensb.frame, 1000);
  sdf.pack.tsdata.sensor[0] = SHT2x.GetTemperature();
  delay(400);
  sdf.pack.tsdata.sensor[1] = SHT2x.GetHumidity();
  delay(400);
  sdf.pack.tsdata.sensor[2] = (float)lightMeter.readLightLevel();
  sdf.pack.tsdata.sensor[3]=(float)rssi;
  LoRa.write(sdf.frame,24);
  LoRa.endPacket();
  Serial.print("Temp:");Serial.println(sdf.pack.tsdata.sensor[0]);
  Serial.print("Humi:");Serial.println(sdf.pack.tsdata.sensor[1]);
  Serial.print("Lumi:");Serial.println(sdf.pack.tsdata.sensor[2]);
  if(sdcount<64000) sdcount++; else sdcount=0;
  //if(!(i%2))
  dispData(1);
  if(i<100) i++; else i=0;  
  if(i==50) dispLabel();                 
  LoRa.receive(); 
  //taskYIELD();
  delay(17700);
  }
}

void onReceive(int packetSize) 
{
int i=0;
uint8_t rdbuff[24], rbbuff[8];  
  if (packetSize == 0) return;   // if there's no packet, return
  i=0;
  if (packetSize==24)
    {
    while (LoRa.available()) {
      rdbuff[i]=LoRa.read();i++;
      }
      //dispData(0); 
      rssi=LoRa.packetRssi();
      if(rdcount<64000) rdcount++; else rdcount=0;
      xQueueReset(dqueue); // to keep only the last element
      xQueueSend(dqueue, rdbuff, portMAX_DELAY); 
    }
  delay(200);
}



void setup() {
  int sf,sb; long freq; int br=0;
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  dispLabel();
  
  introd(&sf,&sb,&freq);

  
  br=bitrate(sf,sb);
  dispDatarate(br,sf,sb);
  
  
  Serial.println(sf);
  Serial.println(sb);
  Serial.println(freq);

  Serial.println(br);


  pinMode(26, INPUT);  // recv interrupt
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  //LoRa.setTxPower(17,PA_OUTPUT_PA_BOOST_PIN);
  //Supported values are between 2 and 17 for PA_OUTPUT_PA_BOOST_PIN, 0 and 14 for PA_OUTPUT_RFO_PIN.
  // Most modules have the PA output pin connected to PA BOOST,
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();
  
 Wire.begin(); 
 lightMeter.begin(); 
 
dqueue = xQueueCreate(8, 24); // queue for 4 LoRaTS data frames
squeue = xQueueCreate(8, 16);  // 4 float
xTaskCreate(
                    LoRaSendTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL);
 
  Serial.println("LoRaSendTask created...");

  LoRa.onReceive(onReceive);
  LoRa.receive();  
  delay(3000);
}

void loop() {
 xQueueReceive(dqueue, rdf.frame, 1000); //portMAX_DELAY)
    dispData(0);
    Serial.println(rdf.pack.tsdata.sensor[0]);
    Serial.println(rdf.pack.tsdata.sensor[1]);
    Serial.println(rdf.pack.tsdata.sensor[2]);
    Serial.println(rdf.pack.tsdata.sensor[3]);
    Serial.println();
    delay(15000);
}
