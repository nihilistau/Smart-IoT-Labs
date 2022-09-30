//Attention low upload speed- 460800

#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

uint64_t chipid;  
uint32_t sendID;
uint32_t recvID;

// with LoRa modem RFM95 and green RFM board - int and RST to solder

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

union 
{
  uint8_t frame[32];
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier chipID (4 lower bytes)
    uint8_t  con[2];    // control field
    char     pass[16];  // password - 16 characters
    uint8_t  pad[6];    // future use
  } pack;               // control packet
} scf,rcf;              // send control frame , receive control frame

int ack=0;

void setup() {
  Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  sendID=(uint32_t)chipid;
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting LoRa Receiver");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  else 
   {
   Serial.println("Starting LoRa ok!");
   LoRa.setSpreadingFactor(sf);
   LoRa.setSignalBandwidth(sbw);
   }
}

int send_ACK()
{
  Serial.println("Sending ACK packet: ");
  // send packet
  LoRa.beginPacket();
  scf.pack.did=(uint32_t)rcf.pack.sid;
  scf.pack.sid=(uint32_t)sendID;
  scf.pack.con[0]=0x01; scf.pack.con[1]=0x00;       // ACK
  strncpy(scf.pack.pass,"passwordpassword",16);   // 16 character password
  LoRa.write(scf.frame,32);
  LoRa.endPacket();
}

int recv()
{
int i=0;
while(1)
  {
  int packetSize = LoRa.parsePacket();
  if (packetSize==32) 
    {
    i=0;
    // received a packet
    Serial.print("Received packet: ");
    // read packet
    while (LoRa.available()) {
      rcf.frame[i] = LoRa.read();i++;
      }
    Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
    Serial.printf("%X\n",rcf.pack.con[1]); 
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    if(rcf.pack.con[1]==0x01 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,"passwordpassword",16))
      {
      Serial.printf("%08X\n",(uint32_t)rcf.pack.did);
      Serial.printf("%s\n",rcf.pack.pass);
      delay(200);
      send_ACK();
      return 1;
      }
    } 
  }   
}


void loop() 
{
int req=0;  
Serial.println("Start recv_REQ");
req=recv();   Serial.println(req);
}
