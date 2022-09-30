
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include "ThingSpeak.h"
// sensitivity table
int senst[5][7] = { // 6     7    8    9   10   11   12   - spreading factor
                    {-125,-129,-133,-136,-138,-142,-143}, //  31250 Hz
                    {-121,-126,-129,-132,-135,-138,-140}, //  62500 Hz
                    {-118,-123,-126,-129,-132,-135,-137}, // 125000 Hz
                    {-115,-120,-123,-126,-129,-132,-134}, // 250000 Hz
                    {-112,-117,-120,-123,-126,-129,-131}, // 500000 Hz
};

char ssid[] = "Livebox-08B0";          //  your network SSID (name) 
char pass[] = "G79ji6dtEptVTPWmZP";   // your network passw
// to be replaced by eduroam ..




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

unsigned long myChannelNumber[8] = {1,4,17,0,0,0,0,0 };  // IoTDevKit1, IoTDevKit2, IoTDevKit3
const char * myWriteAPIKey[8] = { "HEU64K3PGNWG36C4","4M9QG56R7VGG8ONT","MEH7A0FHAMNWJE8P",NULL,NULL,NULL,NULL,NULL};
//  channel names IoTDevKit1, IoTDevKit2, IoTDevKit3
WiFiClient  client;

int rssi;
QueueHandle_t bqueue, dqueue, rssiqueue;  // queues for beacons and data frames

SemaphoreHandle_t xSemRecv,xSemOLED;

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


 union {
   struct 
     {
     int rssi;
     uint8_t dev;
     } pack;
   uint8_t buff_rssi[5];
} rurssi,wurssi;

void dispLabel()
{
  u8x8.clear();
  u8x8.draw2x2String(0,0,"Polytech");
  delay(1000);
  u8x8.draw2x2String(0,2,"Facultes");
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

void dispData(int sr, int rssi, uint8_t dev)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  xSemaphoreTake(xSemOLED,2000);
  u8x8.clear();
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SPacket:%5.5d",sdcount);
  else sprintf(dbuf,"RPacket:%5.5d",rdcount);
  u8x8.drawString(0, 0,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SDevice:%5.5d",(int)sdf.pack.head[1]);
  else sprintf(dbuf,"RDevice:%5.5d",(int)rdf.pack.head[1]);
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
  if(!sr) sprintf(dbuf,"Srssi:%3.3d/%2.2d",(int)rdf.pack.tsdata.sensor[3],(int)rdf.pack.tout);
  u8x8.drawString(0, 6,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"Rrssi:%3.3d/%2.2d",rssi,dev);
  else sprintf(dbuf,"                ");
  u8x8.drawString(0, 7,dbuf);
  if(!sr) delay(3000);
  xSemaphoreGive(xSemOLED);
}




void onReceive(int packetSize) 
{
int i=0,rssi=0;
uint8_t rdbuff[24], rbbuff[8]; 
 xSemaphoreTake(xSemRecv,2000); 
  if (packetSize == 0) return;   // if there's no packet, return
  i=0;
  if (packetSize==24)
    {
    while (LoRa.available()) {
      rdbuff[i]=LoRa.read();i++;
      }
      wurssi.pack.rssi=LoRa.packetRssi();
      wurssi.pack.dev=rdbuff[1];
      xQueueReset(rssiqueue); // to keep only the last element
      xQueueSend(rssiqueue, wurssi.buff_rssi, portMAX_DELAY); 
      if(rdcount<64000) rdcount++; else rdcount=0;
      xQueueReset(dqueue); // to keep only the last element
      xQueueSend(dqueue, rdbuff, portMAX_DELAY); 
    }
   xSemaphoreGive(xSemRecv);
   
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
  //LoRa.setTxPower(14);
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();

dqueue = xQueueCreate(8, 24); // queue for 4 LoRaTS data frames
rssiqueue = xQueueCreate(4, 5);  // queue for rssi and devID

xSemRecv = xSemaphoreCreateMutex();
xSemOLED = xSemaphoreCreateMutex();


   
WiFi.disconnect(true);
delay(1000);
    WiFi.begin(ssid, pass);
Serial.println(WiFi.getMode());
delay(1000);     
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("WiFi setup ok");
  delay(1000);
  Serial.println(WiFi.status());
  ThingSpeak.begin(client);
    delay(1000);
  Serial.println("ThingSpeak begin");
  delay(1000);
  LoRa.onReceive(onReceive);
  LoRa.receive(); 
}



void loop() {
 int rec_rssi,devID;
 xSemaphoreTake(xSemRecv,2000); 
 xQueuePeek(dqueue, rdf.frame, 30000); //portMAX_DELAY)
 xQueuePeek(rssiqueue, rurssi.buff_rssi, 10000);
 Serial.print("RSSI from queue:");Serial.println(rurssi.pack.rssi);
    dispData(0,rurssi.pack.rssi,rurssi.pack.dev);
    Serial.println(rdf.pack.head[2]);
    Serial.println(rdf.pack.tsdata.sensor[0]);
    Serial.println(rdf.pack.tsdata.sensor[1]);
    Serial.println(rdf.pack.tsdata.sensor[2]);
    Serial.println(rdf.pack.tsdata.sensor[3]);
    Serial.println();
    if(rdf.pack.head[2] & 0x80) ThingSpeak.setField(1, rdf.pack.tsdata.sensor[0]);
    if(rdf.pack.head[2] & 0x40) ThingSpeak.setField(2, rdf.pack.tsdata.sensor[1]);
    if(rdf.pack.head[2] & 0x20) ThingSpeak.setField(3, rdf.pack.tsdata.sensor[2]);
    if(rdf.pack.head[2] & 0x10) ThingSpeak.setField(4, rdf.pack.tsdata.sensor[3]);
     

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }
    devID=(int)rdf.pack.head[1];
    ThingSpeak.writeFields(4, "4M9QG56R7VGG8ONT");         

 xSemaphoreGive(xSemRecv); 
     LoRa.receive();
}
