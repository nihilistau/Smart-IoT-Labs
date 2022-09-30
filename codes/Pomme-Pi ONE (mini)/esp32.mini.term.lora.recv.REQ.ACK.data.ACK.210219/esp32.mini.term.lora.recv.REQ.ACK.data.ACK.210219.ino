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

uint16_t timeout;
int drec=1;   // to start the data reqest



union 
{
  uint8_t frame[32];
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier chipID (4 lower bytes)
    uint8_t  con[2];    // control field
    char     pass[16];  // password - 16 characters
    uint16_t tout;      // timeout
    uint8_t  pad[4];    // future use
  } pack;               // control packet
} scf,rcf;              // send control frame , receive control frame

int ack=0;

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

void onReceive(int packetSize) {
  if(packetSize==32)
    {
    for (int i = 0; i < packetSize; i++)  rcf.frame[i]= LoRa.read();
    Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
    Serial.printf("%X,%X\n",rcf.pack.con[0],rcf.pack.con[1]);
    timeout= rcf.pack.tout;
    if(rcf.pack.con[0]==0x01 && rcf.pack.did==sendID) // sent to this node
      {
      recvID = rcf.pack.sid; ack=1;  // received ACK-REQ frame
      Serial.println("Received ACK");
      }
    }
  if(packetSize==64)
    {
    for (int i = 0; i < packetSize; i++)  rdf.frame[i]= LoRa.read();
    Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
    Serial.printf("%X,%X\n",rdf.pack.con[0],rdf.pack.con[1]); 
    if(rdf.pack.con[0]==0x03 && rdf.pack.did==sendID) // sent to this node error frame
      {
      Serial.println("reception error on TS server"); timeout=0; drec=1;
      }
    if(rdf.pack.con[0]==0x04 && rdf.pack.did==sendID) // sent to this node good frame 
      { 
      Serial.println(rdf.pack.channel);Serial.println(rdf.pack.keyword);
      Serial.println("Received requested data fields:");
      for(int i=0;i<8;i++) { Serial.print(rdf.pack.sens[i]);Serial.print(","); }

      drec=1;
      }
    }
  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}


void setup() {
  Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  sendID=(uint32_t)chipid;
  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting LoRa Sender");
  Serial.printf("%08X\n",(uint32_t)chipid);
  // Initialising the UI will init the display too.

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  else 
   {
   Serial.println("Starting LoRa ok!");
   LoRa.setSpreadingFactor(sf);
   LoRa.setSignalBandwidth(sbw);

  // register the receive callback
  LoRa.onReceive(onReceive);
      LoRa.receive(); 
   }
}

int counter=0;
char buff[32];

void send_REQ()
{
  Serial.println("Sending AC REQ packet: ");
  // send packet
  LoRa.beginPacket();
  scf.pack.did=(uint32_t)0;
  scf.pack.sid=(uint32_t)sendID;
  scf.pack.con[0]=0x00; scf.pack.con[1]=0x01;       // ID request
  strncpy(scf.pack.pass,"passwordpassword",16);   // 16 character password
  LoRa.write(scf.frame,32);
  LoRa.endPacket();
}

void send_Data_REQ(char *rkey, uint8_t mask, int chan, float *stab)  // write key , sensor mask, sensor table values
{
  Serial.println("Sending data request packet: ");
  // send packet
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)recvID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)sendID;
  sdf.pack.con[0]=0x02; sdf.pack.con[1]=mask;       // send reqest for data
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,rkey,16);
  sdf.pack.tout=0;
  LoRa.write(sdf.frame,64);
  LoRa.endPacket();
}


void loop() {
  if(ack==0) 
    { 
    send_REQ();  Serial.println("Set recv mode");
    LoRa.receive(); 
    }
  if(drec)  // data received
    {
    char *rkey="0XYA1MAWXFGVWDX9";
    int chan=1243348;  // my_channel
    float stab[8]={1.0,2.1,3.2,4.3,5.4,6.5,7.6,8.7};
    uint8_t mask=0xC0;
    delay(400);
    send_Data_REQ(rkey, mask, chan, stab); 
    drec=0;Serial.println("Data request sent"); 
    LoRa.receive(); 
    }
 delay(5000);

}
