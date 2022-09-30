#include <WiFi.h> 
#include "ThingSpeak.h"
#include <SPI.h>             
#include <LoRa.h>
#include <aes256.h>
#include <U8x8lib.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    869E6  //you can set band here directly,e.g. 868E6,915E6
#define sf 9
#define sb 250E3       // set the signal bandwidth; 125KHz,[250KHz,500Kz]

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);

aes256_context ctxt;

  uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };

struct esp_aes_context *aescont;

float flat, flon;

char ssid[] = "YotaPhoneAP";          //  your network SSID (name) 
char pass[] = "tonbridge";   // your network passw

unsigned long myChannelNumber[8] = {1,4,17,0,0,0,0,0 };  // IoTDevKit1, IoTDevKit2, IoTDevKit3
const char * myWriteAPIKey[8] = { "HEU64K3PGNWG36C4","4M9QG56R7VGG8ONT","MEH7A0FHAMNWJE8P",NULL,NULL,NULL,NULL,NULL};
//  channel names IoTDevKit1, IoTDevKit2, IoTDevKit3
WiFiClient  client;
int i=0,rssi;
int counter=0;

QueueHandle_t dqueue;  // queues for beacons and data frames

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

void dispData(uint8_t ter)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  u8x8.drawString(0, 0,"AES LoRaTS Link");
  memset(dbuf,0x00,32);
  if(ter==0x01) sprintf(dbuf,"T1-Temp:%2.2f",rdf.pack.sensor[0]);
  if(ter==0x02) sprintf(dbuf,"T2-Temp:%2.2f",rdf.pack.sensor[2]);
//  if(ter==0x03) sprintf(dbuf,"T3-Temp:%2.2f",rdf.pack.sensor[4]);
//  if(ter==0x04) sprintf(dbuf,"T4-Temp:%2.2f",rdf.pack.sensor[6]);
  u8x8.drawString(0, 2,dbuf);
  memset(dbuf,0x00,32);
  if(ter==0x01) sprintf(dbuf,"T1-Humi:%2.2f",rdf.pack.sensor[1]);
  if(ter==0x02) sprintf(dbuf,"T2-Humi:%2.2f",rdf.pack.sensor[3]);
//  if(ter==0x03) sprintf(dbuf,"T3-Humi:%2.2f",rdf.pack.sensor[5]);
//  if(ter==0x04) sprintf(dbuf,"T3-Humi:%2.2f",rdf.pack.sensor[7]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"RSSI:%4.4d",rssi);
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



void onReceive(int packetSize) 
{
uint8_t rdbuff[24], rbbuff[8];  
  if (packetSize == 0) return;   // if there's no packet, return
  i=0;
  if (packetSize==24)
    {
    while (LoRa.available()) {
      rdbuff[i]=LoRa.read();
      i++;
      } 
      rssi=LoRa.packetRssi();
      Serial.println("got");
      xQueueSend(dqueue, rdbuff, portMAX_DELAY); 
    }
}



void SendTSTask( void * pvParameters )
{
while(1)
  {    
    xQueueReceive(dqueue, rdf.frame, portMAX_DELAY);  //  default: portMAX_DELAY
    // decryption of 16-byte block - short data packet payload
    aes256_decrypt_ecb(&ctxt,(rdf.frame+8));
    delay(200);
    Serial.println((float)rdf.pack.sensor[0]);
    dispData(rdf.pack.head[1]);                          // increment message ID
    delay(200);
    Serial.println(counter);counter++;
    if(rdf.pack.head[1]==0x01 || rdf.pack.head[1]==0x02)
    {
        if(rdf.pack.head[2] & 0x80) ThingSpeak.setField(1, rdf.pack.sensor[0]);
        if(rdf.pack.head[2] & 0x40) ThingSpeak.setField(2, rdf.pack.sensor[1]);
        if(rdf.pack.head[2] & 0x20) ThingSpeak.setField(3, rdf.pack.sensor[2]);
        if(rdf.pack.head[2] & 0x10) ThingSpeak.setField(4, rdf.pack.sensor[3]); 
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
          }
        ThingSpeak.writeFields(myChannelNumber[1], myWriteAPIKey[1]); 
    }           
    LoRa.receive();
  }  
} 

void setup() {
  Serial.begin(9600);                   // initialize serial
  pinMode(26, INPUT);  // recv interrupt
 
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.println("LoRaTS gateway");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  dispPara();
  delay(1000);
  Serial.println("LoRa init succeeded.");
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

  dqueue = xQueueCreate(4, 24); // queue for 4 LoRaTS data frames
  
     xTaskCreate(
                    SendTSTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL     );  /* Task handle */
 
  Serial.println("SendTSTask created...");
  LoRa.onReceive(onReceive);
  LoRa.receive();  
  aes256_init(&ctxt, key);
  delay(3000);
}


void loop() {
}



