//Attention low upload speed- 460800

#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
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

int ack=0, sub=0, dack=0;  // received ack (ID REQ), subscription, data

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

void onReceive(int packetSize) {
  if(packetSize==32)
    {
    for (int i = 0; i < packetSize; i++) 
    rcf.frame[i]= LoRa.read();
    Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
    Serial.printf("%X,%X\n",rcf.pack.con[0],rcf.pack.con[1]);
    if(rcf.pack.con[0]==0x01 && rcf.pack.did==sendID)   // ACK for ID REQ sent to this node
      {
      recvID = rcf.pack.sid; ack=1;
      }
    if(rcf.pack.con[0]==0x02 && rcf.pack.did==sendID)   // ACK for subscribe sent to this node
      {
      sub=1;
      }
    }
  if(packetSize==64)
    {
    dack=0;
    for (int i = 0; i < packetSize; i++) 
    rdf.frame[i]= LoRa.read();
    if(rdf.pack.con[1]==0x20 )   // Data for subscribed topic sent to this node
      {
      dack=1;  // do something with the received data
      Serial.println(rdf.pack.topic);Serial.println(rdf.pack.mess);
      }
  // print RSSI of packet
    }
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}

void dispTask( void * parameter )
{
char buff[32];  
for(;;)
  {
  Serial.println("this is display Task");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);memset(buff,0x00,32);
  strncpy(buff,rdf.pack.mess,16);
  display.drawString(0,0,buff);
  display.display();
  delay(4000);
  }
}


void setup() {
  Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  sendID=(uint32_t)chipid;
  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Hello LoRa");
  display.display();
  
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
   xTaskCreate(
          dispTask, /* Task function. */
          "display Task", /* name of task. */
          10000, /* Stack size of task */
          NULL, /* parameter of the task */
          1, /* priority of the task */
          NULL); /* Task handle to keep track of created task */
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

void subscribe(char *topic)
{
  Serial.println("Sending Publish packet: ");
  // send packet
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)recvID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)sendID;
  sdf.pack.con[0]=0x00; sdf.pack.con[1]=0x20;       // subscribe data
  strcpy(sdf.pack.topic,topic);   // topic name
  LoRa.write(sdf.frame,64);
  LoRa.endPacket();
}


void loop() {
  if(ack==0) 
    { 
    send_REQ();  Serial.println("Set recv mode");
    LoRa.receive(); 
    }
  else
    {
    char topic[24],message[24];
    strcpy(topic,"/esp32/Term1/Sens1");
    subscribe(topic);  Serial.println("Topic subscribed"); 
    LoRa.receive();  
    }
  delay(20000);  // waiting for the message on the subscribed topic
}
