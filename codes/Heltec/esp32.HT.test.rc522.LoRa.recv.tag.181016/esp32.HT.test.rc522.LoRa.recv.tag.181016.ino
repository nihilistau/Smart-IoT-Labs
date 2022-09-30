
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

#define SDA_PIN 21
#define SCL_PIN 22
#define RST_PIN 12

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    870E6  //you can set band here directly,e.g. 868E6,915E6
int  sf=8;
int sb=125E3;       // set the signal bandwidth; 125KHz,[250KHz,500Kz]

int rssi;



QueueHandle_t bqueue, dqueue;  // queues for beacons and data frames

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

char *namet[4][3] = { 
                      { "0E F6 BE 23","Przemyslaw","Bakowski" },
                      { "1E F6 BE 23", "Catherine", "Galais" },
                      { "2E F6 BE 23", "Jean", "Moulin" },
                      { "3E F6 BE 23","John", "Smith" }
                    };

int  findName(char* tag)   
{
  int i=0;
  for(i=0;i<4;i++)
    {                   
    if(!strncmp(tag,namet[i][0],11)) return i; // tag found
    }
return -1; // no match
}

void dispName()  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  u8x8.drawString(0, 1,"Send name");
  strncpy(dbuf, sdf.pack.tsdata.text,16);
  u8x8.drawString(0, 2,dbuf);
  strncpy(dbuf, sdf.pack.tsdata.text+16,16);
  u8x8.drawString(0, 3,dbuf);
}


void onReceive(int packetSize) 
{
  int i=0;uint8_t rdbuff[24];  
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
  Serial.begin(9600); // Initialize serial communications with the PC

  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  pinMode(26, INPUT);  // recv interrupt
  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  dqueue = xQueueCreate(8, 24); // queue for 4 LoRaTS data frames

  LoRa.onReceive(onReceive);
  LoRa.receive();  
  delay(3000);
}


void loop() {
  
int ind=-1;int qres=0;
int i=0;
Serial.println("on queue");

memset(rdf.frame,0x00,24);  // short data frame
  
qres=xQueueReceive(dqueue, rdf.frame, 10000); //portMAX_DELAY) - wating max 10 s

Serial.println(qres);
Serial.println((int)rdf.pack.head[0]);
Serial.println((int)rdf.pack.head[1]);
Serial.println((int)rdf.pack.head[2]);
Serial.println((int)rdf.pack.head[3]);

for(i=0;i<16;i++)
    Serial.print(rdf.pack.tsdata.text[i],HEX);

if(qres)
  {
  Serial.println(rdf.pack.tsdata.text);
  ind=findName(rdf.pack.tsdata.text);
  Serial.println(ind);
//  LoRa.beginPacket();
//  memset(sdf.frame,0x00,40);
//  sdf.pack.head[0]= 0x02;  // destination term 1
//  sdf.pack.head[1]= 0x01;  // source term 2
//  sdf.pack.head[2]= 0x80;  // field mask - filed1 set
//  sdf.pack.head[3]= 0xa0;  // data size 32
//  sdf.pack.num= (uint16_t) 0; // timeout 0
//  if(ind<0)
//    {
//    strcpy(sdf.pack.tsdata.text,"toto");    // first name
//    strcpy(sdf.pack.tsdata.text+16,"toto"); // family name
//    }
//  else  
//    {
//    strcpy(sdf.pack.tsdata.text,namet[ind][1]);    // first name
//    strcpy(sdf.pack.tsdata.text+16,namet[ind][2]); // family name
//    }
//      dispName();
//  LoRa.write(sdf.frame,40);  // long frame
//  LoRa.endPacket();
//  }
//  LoRa.receive(); 
  delay(100);
  }
}


