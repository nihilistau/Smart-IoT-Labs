#include <WiFi.h> 
#include "ThingSpeak.h"
#include <SPI.h>             
#include <LoRa.h>
#include <aes256.h>
#include <U8x8lib.h>
#include <TinyGPS.h>

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

aes256_context ctxt;

  uint8_t key[] = { //
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
  };

TinyGPS gps;
HardwareSerial ss(2); 

float flat, flon;

char ssid[] = "YotaPhoneAP";          //  your network SSID (name) 
char pass[] = "tonbridge";   // your network passw

unsigned long myChannelNumber[8] = {1,4,17,0,0,0,0,0 };  // IoTDevKit1, IoTDevKit2, IoTDevKit3
const char * myWriteAPIKey[8] = { "HEU64K3PGNWG36C4","4M9QG56R7VGG8ONT","MEH7A0FHAMNWJE8P",NULL,NULL,NULL,NULL,NULL};
//  channel names IoTDevKit1, IoTDevKit2, IoTDevKit3
WiFiClient  client;
int i=0,rssi;

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
  delay(200);
}

void dispGPS(int sat,float flat,float flon, char *date, char *time)
{
  char dbuf[16];
  u8x8.clear();
  Serial.println("GPS coordinates");
  u8x8.drawString(0, 1, "GPS coordinates");
  sprintf(dbuf,"SatNum:%d",sat);
  u8x8.drawString(0, 2, dbuf);
  sprintf(dbuf,"Lat:%f",flat);
  u8x8.drawString(0, 3,dbuf);
  sprintf(dbuf,"Long:%f",flon);
  u8x8.drawString(0, 4, dbuf);
  u8x8.drawString(0, 6, date);
  u8x8.drawString(0, 7, time);
  delay(3000);
}


void SendTSTask( void * pvParameters )
{
while(1)
  {    
    xQueueReceive(dqueue, rdf.frame, portMAX_DELAY);  //  default: portMAX_DELAY
    // decryption of 16-byte block - short data packet payload
    aes256_decrypt_ecb(&ctxt,(rdf.frame+8));
    dispData(0);                           // increment message ID
    delay(200);
    if(rdf.pack.head[1]==0x01 || rdf.pack.head[1]==0x02)
    {
        if(rdf.pack.head[2] && 0x80) ThingSpeak.setField(1, rdf.pack.sensor[0]);
        if(rdf.pack.head[2] && 0x40) ThingSpeak.setField(2, rdf.pack.sensor[1]);
        if(rdf.pack.head[2] && 0x20) ThingSpeak.setField(3, rdf.pack.sensor[2]);
        if(rdf.pack.head[2] && 0x10) ThingSpeak.setField(4, rdf.pack.sensor[3]); 
        ThingSpeak.setField(5, flat);
        ThingSpeak.setField(6, flon);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
          }
        ThingSpeak.writeFields(myChannelNumber[1], myWriteAPIKey[1]); 
    }           
    delay(200); 
    LoRa.receive();
  }  
} 

void setup() {
  Serial.begin(9600);                   // initialize serial
  pinMode(26, INPUT);  // recv interrupt
  ss.begin(9600, SERIAL_8N1, 17, 16);
  Serial.println("start GPS");
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
  WiFi.disconnect(true); // reset credentials
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
  
     xTaskCreate(
                    SendTSTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL     );  /* Task handle */
 
  Serial.println("SendTSTask created...");
  LoRa.onReceive(onReceive);
  LoRa.receive();  delay(3000);
}


void loop() {
bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  for (unsigned long start = millis(); millis() - start < 20000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      //Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
  if (newData)
  {
    unsigned long age;
    unsigned long ldate,ltime,tage;
    char sdate[24],stime[24],sstime[24],gpsbuff[64];
    gps.f_get_position(&flat, &flon, &age);
    gps.get_datetime(&ldate,&ltime,&tage);
    sprintf(sdate,"Date:%ld",ldate); //Serial.println(sdate);
    sprintf(stime,"GMT: %ld",ltime); //Serial.println(sdate);
    Serial.println(sdate);
    Serial.println(stime);
    memset(sstime,0x00,24);
    if(strlen(stime)==13)
        strncpy(sstime,stime,11);
    else { strncpy(sstime,"GMT: 0",6); strncat(sstime,stime+5,5); }    
    Serial.println(sstime);  
    memset(gpsbuff,0x00,64);
    sprintf(gpsbuff,"Sat:%d  Lat:%2.6f Long:%2.6f", gps.satellites(),flat,flon);
    Serial.println(gpsbuff);
    dispGPS(gps.satellites(),flat,flon,sdate,sstime);   
  }
}



