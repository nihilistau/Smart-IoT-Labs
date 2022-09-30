//Attention low upload speed- 460800
#include <WiFi.h>
#include <MQTT.h>
#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";
const char* mqttServer = "broker.emqx.io";
WiFiClient net;
MQTTClient client;

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
    uint16_t tout;      // timeout for next data frame
    uint8_t  pad[4];    // future use
  } pack;               // control packet
} scf,rcf;              // send control frame , receive control frame

int ack=0;

union 
{
  uint8_t frame[64];    // MQTT frame to publish on the given topic
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier chipID (4 lower bytes)
    uint8_t  con[2];    // control field
    char     topic[24];  // topic name - e.g. /esp32/Term1/Sens1
    char     mess[24];   // message value
    uint8_t  pad[6]; // future use
  } pack;               // data packet
} sdf,rdf;              // send data frame , receive data frame

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect("IoT.GW1")) {
    Serial.print("."); delay(1000);
  }
  Serial.println("\nIoT.GW1 - connected!");
}


void setup() {
  Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  sendID=(uint32_t)chipid;

  WiFi.begin(ssid, pass);
  client.begin(mqttServer, net);
  connect();
  
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

int send_ACK(uint8_t idrd,uint16_t timeout)
{
  Serial.print("Sending ACK packet with tout: ");Serial.println(timeout);
  // send packet
  LoRa.beginPacket();
  scf.pack.did=(uint32_t)rcf.pack.sid;
  scf.pack.sid=(uint32_t)sendID;
  scf.pack.con[0]=idrd; scf.pack.con[1]=0x00;       // ACK  - 0x01 for IDREQ ACK and 0x02 for DataACK
  strncpy(scf.pack.pass,"passwordpassword",16);   // 16 character password
  scf.pack.tout= timeout;
  LoRa.write(scf.frame,32);
  LoRa.endPacket();
}

int packetSize=0;
uint16_t tout=1000;

void loop() 
{
int i=0;
client.loop();  
packetSize = LoRa.parsePacket();
if (packetSize==32) 
  {
  i=0;
  // received control packet ?
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
    send_ACK(0x01,tout);  // ID request ACK or Data ACK - control field, timeout not valid
    }
  } 
if (packetSize==64)
  {
  i=0;
  Serial.println("received publish packet ?");
  while (LoRa.available()) {
    rdf.frame[i] = LoRa.read();i++;
    }
  if(rdf.pack.con[1]==0x10 && rdf.pack.did==sendID)
    {
    Serial.printf("%08X\n",(uint32_t)rdf.pack.did);
    Serial.printf("%s\n",rdf.pack.topic);
    Serial.printf("%s\n",rdf.pack.mess);
     if (!client.connected()) { connect(); }
    client.publish(rdf.pack.topic, rdf.pack.mess);
    send_ACK(0x02,tout);  // to be used or onot - e.g. timeout
    delay(200); 
    }
  }
packetSize=0;
}

  
