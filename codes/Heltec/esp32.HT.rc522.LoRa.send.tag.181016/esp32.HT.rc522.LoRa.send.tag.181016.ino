
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

#include "MFRC522_I2C.h"

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

MFRC522 mfrc522(0x28, RST_PIN);  // Create MFRC522 instance.

void ShowReaderDetails();


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


void dispName()  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  u8x8.drawString(0, 1,"Received name");
  strncpy(dbuf, rdf.pack.tsdata.text,16);
  u8x8.drawString(0, 2,dbuf);
  strncpy(dbuf, rdf.pack.tsdata.text+16,16);
  u8x8.drawString(0, 3,dbuf);
}

void dispTag(char *rtag)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  u8x8.drawString(0, 1,"Received tag");
  u8x8.drawString(0, 2,rtag);
}


void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}



void onReceive(int packetSize) 
{
  int i=0;uint8_t rdbuff[40];  
  if (packetSize == 0) return;   // if there's no packet, return
  i=0;
  if (packetSize==40)
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
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C
  mfrc522.PCD_Init();   // Init MFRC522
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println("Scan PICC to see UID, type, and data blocks...");
  
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

dqueue = xQueueCreate(4, 40); // queue for 4 LoRaTS long data frames

  LoRa.onReceive(onReceive);
  LoRa.receive();  
  delay(3000);
}

char tbuff[16];

int period = 3000;

unsigned long time_now = 0;

void loop() {
int qres=0;
  // Look for new cards
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
 //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();

  Serial.print("Message : ");
  content.toUpperCase();
  memset(tbuff,0x00,16);
  String textstr=content.substring(1);
  textstr.toCharArray(tbuff,12);
  Serial.println(tbuff);
  Serial.println(strlen(tbuff));

  dispTag(tbuff);

//  LoRa.beginPacket();
//  memset(sdf.frame,0x00,24);
//  sdf.pack.head[0]= 0x01;  // destination term 1
//  sdf.pack.head[1]= 0x02;  // source term 2
//  sdf.pack.head[2]= 0x80;  // field mask - filed1 set
//  sdf.pack.head[3]= 0x90;  // data size 16 bytes in text - RFID tag - MSB set
//  sdf.pack.num= (uint16_t) sdcount;
//  sdf.pack.num= (uint16_t) 0; // timeout 0
//  strncpy(sdf.pack.tsdata.text,tbuff,12);  // "0E F6 BE 23"
//  delay(200);
//  Serial.println(sdf.pack.tsdata.text);
//  delay(200);
//  LoRa.write(sdf.frame,24);
//  LoRa.endPacket();
//
//  LoRa.receive(); 
  memset(rdf.frame,0x00,40); // long data frame
  qres=xQueueReceive(dqueue, rdf.frame, 10000); //portMAX_DELAY) - wating max 10 s
  Serial.println(qres);
  Serial.println(rdf.pack.tsdata.text);

  dispName();

  delay(1000);
}


