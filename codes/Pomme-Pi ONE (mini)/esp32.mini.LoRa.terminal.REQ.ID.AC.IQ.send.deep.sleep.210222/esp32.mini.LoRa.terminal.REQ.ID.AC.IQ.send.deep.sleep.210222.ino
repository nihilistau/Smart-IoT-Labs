
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

uint64_t chipid;  
uint32_t nodeID;  // local ID on 4 bytes
//uint32_t recvID;  // gateway ID

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

uint16_t timeout=15;
int data_ack=0;
RTC_DATA_ATTR int idreq_ack=0;
RTC_DATA_ATTR uint32_t recvID=0;


union 
{
  uint8_t frame[32];
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier nodeID (4 lower bytes)
    uint8_t  con[2];    // control field
    char     pass[16];  // password - 16 characters
    uint16_t tout;      // timeout
    uint8_t  pad[4];    // future use
  } pack;               // control packet
} scf,rcf;              // send control frame , receive control frame

union 
{
  uint8_t frame[64];    // TS frame to send/receive data
  struct
  {
    uint32_t did;        // destination identifier chipID (4 lower bytes)
    uint32_t sid;        // source identifier chipID (4 lower bytes)
    uint8_t  con[2];     // control field: lower byte is used as mask
    int     channel;     // TS channel number
    char    keyword[16];   // write (or read) keyword
    float   sens[8];     // max 8 values – fields
    uint16_t  tout;      // optional timeout
  } pack;                // data packet
} sdf,rdf; 



void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  nodeID=(uint32_t)chipid; 
  Serial.printf("\nChipID:%08X\n",(uint32_t)nodeID);
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);
  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.printf("%08X\n",(uint32_t)chipid);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa ok!");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw); 
  LoRa.setSyncWord(0xF3);           // ranges from 0-0xFF, default 0x34, see API docs
  Serial.println("Tx: invertIQ disable - Rx: invertIQ enable");  
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() 
{
  if (runEvery(3000)) 
    { // repeat every 1000 millis
    if(idreq_ack==0) 
      {
      send_IDREQ(); // send IDREQ control frame
      Serial.println("IDREQ sent");
      }
    else
    {
    if(data_ack==0)
      {  
      char *wkey="J4K8ZIWAWE8JBIX7";
      int chan=1243348;  // my_channel
      float stab[8]={1.0,2.1,3.2,4.3,5.4,6.5,7.6,8.7};
      uint8_t mask=0xC0;  
      send_DATA(mask,chan,wkey,stab); // send DATA frame
      Serial.printf("DATA sent to Gateway: %08X\n",recvID);
      }
    if(data_ack==1) 
    {
      LoRa.end();  // stops LoRa modem and SPI bus connection
      if(timeout==0) timeout=15;   // if received timeout is 0 set 15 secondes
      Serial.printf("Enters deep sleep for: %d sec\n",timeout); 
      esp_sleep_enable_timer_wakeup(1000*1000*timeout);  // in micro-seconds - timeout in seconds
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); delay(100);
      esp_deep_sleep_start(); 
    }
   }
  }
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void send_IDREQ()
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)nodeID;
  scf.pack.con[0]=0x01;scf.pack.con[1]=0x00;
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}


void get_DATA(float *stab)
{
for(int i=0;i<8;i++) sdf.pack.sens[i]=stab[i];
}

void send_DATA(uint8_t mask, int chan, char *wkey, float *stab)
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)recvID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)nodeID;
  sdf.pack.con[0]=0x03; sdf.pack.con[1]=mask;       // send reqest for data
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,wkey,16);
  get_DATA(stab);
  sdf.pack.tout=0;
  LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x02 && rcf.pack.did==nodeID)  // received ACK_ID frame for this node  
      {
      idreq_ack=1; recvID=rcf.pack.sid; 
      }
    if(rcf.pack.con[0]==0x04 && rcf.pack.did==nodeID)  // received DATACK frame for this node  
      {
      data_ack=1; recvID=rcf.pack.sid; timeout=rcf.pack.tout;
      } 
    Serial.printf("Terminal:%08X, ACK from Gateway:%08X\n",(uint32_t)nodeID,(uint32_t)recvID);
    Serial.printf("RSSI: %d\n",LoRa.packetRssi());
    }
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
