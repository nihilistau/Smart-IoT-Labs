
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset
HTU21D TempHumi;

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


void dispData(int sr)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SPacket:%4.4d",sdcount);
  else sprintf(dbuf,"RPacket:%4.4d",rdcount);
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
  if(!sr) sprintf(dbuf,"Srssi:%3.3d",(int)sdf.pack.tsdata.sensor[2]);
  u8x8.drawString(0, 6,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"Rrssi:%3.3d",rssi);
  else sprintf(dbuf,"                ");
  u8x8.drawString(0, 7,dbuf);
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

float humi,temp;

void sensors()
{
  delay(1000);
  humi = TempHumi.readHumidity();
  temp = TempHumi.readTemperature();
}



void LoRaSendTask( void * pvParameters )
{
TickType_t xLastWakeTime;
const TickType_t xFrequency = 2000;  // 1 tick 100ms
xLastWakeTime = xTaskGetTickCount();
while(1)
  {  
     vTaskDelayUntil( &xLastWakeTime, xFrequency );

  LoRa.beginPacket();
  sdf.pack.head[0]= 0x01;  // destination term 1
  sdf.pack.head[1]= 0x02;  // source term 2
  sdf.pack.head[2]= 0xe0;  // field mask - filed1,field2, field3
  sdf.pack.head[3]= 0x10;  // data size 16 bytes
  sdf.pack.num= (uint16_t) sdcount;
  sdf.pack.num= (uint16_t) 0; // timeout 0
  sdf.pack.tsdata.sensor[0]=TempHumi.readTemperature();;
  sdf.pack.tsdata.sensor[1]=TempHumi.readHumidity();  // data from IR receiver
  sdf.pack.tsdata.sensor[2]=(float)rssi;
  LoRa.write(sdf.frame,24);
  LoRa.endPacket();
  Serial.print("Temp:");Serial.println(sdf.pack.tsdata.sensor[0]);
  Serial.print("Humi:");Serial.println(sdf.pack.tsdata.sensor[1]);
  if(sdcount<64000) sdcount++; else sdcount=0;
                     // wait for 20 seconds
  dispData(1);                   
  LoRa.receive(); 
  taskYIELD();
  }
}


int i=0;
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

  TempHumi.begin();
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

char textbuff[64];

int period = 3000;

unsigned long time_now = 0;

void loop() {

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
  String textstr=content.substring(1);
  
  if(content.substring(1) == "0E F6 BE 23" || content.substring(1) == "1E F6 BE 23")
  {
    Serial.println("Authorized access");
    dispData(0);
    xQueueReceive(dqueue, rdf.frame, 1000); //portMAX_DELAY)
    Serial.println(rdf.pack.tsdata.sensor[0]);
    Serial.println(rdf.pack.tsdata.sensor[1]);
    Serial.println(rdf.pack.tsdata.sensor[2]);
    Serial.println();
    delay(120E3);
  }
  else 
  {
  u8x8.clear();
  u8x8.drawString(0, 1,"No access");
  }
  delay(1000);
}


