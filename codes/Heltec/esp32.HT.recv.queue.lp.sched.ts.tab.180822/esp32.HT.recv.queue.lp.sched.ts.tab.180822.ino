#include <WiFi.h> 
#include "ThingSpeak.h"
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
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);

char ssid[] = "YotaPhoneAP";          //  your network SSID (name) 
char pass[] = "tonbridge";   // your network passw

unsigned long myChannelNumber[8] = {1,4,17,0,0,0,0,0 };  // IoTDevKit1, IoTDevKit2, IoTDevKit3
const char * myWriteAPIKey[8] = { "HEU64K3PGNWG36C4","4M9QG56R7VGG8ONT","MEH7A0FHAMNWJE8P",NULL,NULL,NULL,NULL,NULL};
//  channel names IoTDevKit1, IoTDevKit2, IoTDevKit3
WiFiClient  client;
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

const int TerNum=2;
int cycle=30*TerNum;


void SendBeaconTSTask( void * pvParameters )
{
int cyctime,timeout=30;  
while(1)
  {    
    xQueueReceive(dqueue, rdf.frame, portMAX_DELAY);
    dispData(0); 
    cyctime= (millis()/1000)%cycle;
    Serial.println(cyctime);
    Serial.println("Beacon sent");
    LoRa.beginPacket();                   // start packet
    sbf.pack.head[0]=rdf.pack.head[1];    // set destination address from received
    sbf.pack.head[1]=0xff;                 // set sender address - gateway
    sbf.pack.head[2]=0x01;                 // set data type: sensors - timeout
    sbf.pack.head[3]=0x00;                 // set payload length: 16 bytes - beacon
    sbf.pack.num=(uint16_t) sbcount;        // set data packet count
    timeout=cycle-cyctime+30*((int)(rdf.pack.head[1] -1));
    sbf.pack.tout=(uint16_t) timeout;            // set timeout                         
    LoRa.write(sbf.frame,8);              // load beacon frame
    LoRa.endPacket();        // finish packet and send it
    delay(200);
    LoRa.beginPacket();
    LoRa.write(sbf.frame,8);
    LoRa.endPacket();   
    dispBeacon();
    sbcount++;                           // increment message ID
    delay(200);
    
        //if(rdf.pack.head[2] && 0x80) ThingSpeak.setField(1, rdf.pack.sensor[0]);
        ThingSpeak.setField(1, (float)rdf.pack.num);
        if(rdf.pack.head[2] && 0x40) ThingSpeak.setField(2, rdf.pack.sensor[1]);
        if(rdf.pack.head[2] && 0x20) ThingSpeak.setField(3, rdf.pack.sensor[2]);
        if(rdf.pack.head[2] && 0x10) ThingSpeak.setField(4, rdf.pack.sensor[3]);
        //if(rdf.pack.head[2] && 0x08) ThingSpeak.setField(5, rdf.pack.sensor[4]);
        ThingSpeak.setField(5, (float)rssi);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
          }
        ThingSpeak.writeFields(myChannelNumber[1], myWriteAPIKey[1]);        
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

  delay(1000);
    WiFi.begin(ssid, pass);
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
  bqueue = xQueueCreate(4, 8); // queue for 4 LoRaTS beacon frames
  
     xTaskCreate(
                    SendBeaconTSTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL     );  /* Task handle */
 
  Serial.println("SendBeaconTSTask created...");
  LoRa.onReceive(onReceive);
  LoRa.receive();  delay(3000);
}



void loop() 
{
delay(15000);
}




