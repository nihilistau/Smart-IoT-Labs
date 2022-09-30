#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "ThingSpeak.h" 
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL
QueueHandle_t dqueue, tsqueue;
int queueSize = 32;
static int taskCore = 0;

uint64_t chipid;  
uint32_t nodeID;
uint32_t recvID;

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
char *gtpass="passwordpassword";

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

typedef union 
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
} dataframe_t; 

dataframe_t sdf,rdf;

void dispTask( void * pvParameters ){
float stab[8]; char dbuff[32];
while(true)
  {
  xQueueReceive(dqueue, stab, portMAX_DELAY);
  Serial.printf("Sensor1:%2.2f, Sensor2:%2.2f\n",stab[0],stab[1]);
  sprintf(dbuff,"%T:%2.2f  H:%2.2f",stab[0],stab[1]);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Hello world");
  display.drawString(0, 16, dbuff);
  display.display();
  delay(1000);
  }
}


void TS_Task( void * pvParameters ){
float stab[8]; char dbuff[32];
dataframe_t rdf;
while(true)
  {
  xQueueReceive(tsqueue, &rdf, portMAX_DELAY);
 if(rdf.pack.con[1] & 0x80) ThingSpeak.setField(1, rdf.pack.sens[0]);
  if(rdf.pack.con[1] & 0x40) ThingSpeak.setField(2, rdf.pack.sens[1]);
  if(rdf.pack.con[1] & 0x20) ThingSpeak.setField(3, rdf.pack.sens[2]);
  if(rdf.pack.con[1] & 0x10) ThingSpeak.setField(4, rdf.pack.sens[3]);
  if(rdf.pack.con[1] & 0x08) ThingSpeak.setField(5, rdf.pack.sens[4]);
  if(rdf.pack.con[1] & 0x04) ThingSpeak.setField(6, rdf.pack.sens[5]);
  if(rdf.pack.con[1] & 0x02) ThingSpeak.setField(7, rdf.pack.sens[6]);
  if(rdf.pack.con[1] & 0x01) ThingSpeak.setField(8, rdf.pack.sens[7]);

  Serial.println(rdf.pack.con[1]);
  Serial.println(rdf.pack.keyword);
  Serial.println(rdf.pack.sens[0]);
  Serial.println(rdf.pack.sens[1]);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields((uint32_t)rdf.pack.channel, rdf.pack.keyword); // "J4K8ZIWAWE8JBIX7"
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(1000);
  }
}



void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  nodeID=(uint32_t)chipid;
  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting LoRa Sender");
  Serial.printf("%08X\n",(uint32_t)chipid);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa ok!");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Simple Gateway");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  dqueue = xQueueCreate( queueSize, sizeof(float)*8);
  tsqueue = xQueueCreate( queueSize, sizeof(rdf));
  xTaskCreatePinnedToCore(
                    dispTask,   /* Function to implement the task */
                    "dispTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");

   xTaskCreatePinnedToCore(
                    TS_Task,   /* Function to implement the task */
                    "TS_Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}

void loop() {
  if (runEvery(5000)) { // repeat every 5000 millis
  Serial.println("In the gateway");
  }
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void send_ACK(uint8_t con)             // ACK frame with specific control field
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  if(con==0x02) scf.pack.did=(uint32_t)rcf.pack.sid;  // ack for control frame
  if(con==0x04) scf.pack.did=(uint32_t)rdf.pack.sid;  // ack for data frame
  scf.pack.sid=(uint32_t)nodeID;  // send ACK frame with the received nodeID
  scf.pack.con[0]=con;scf.pack.con[1]=0x00;
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.print("ACK sent with: ");Serial.println(con,HEX);
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x01 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ frame
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      Serial.print("Gateway ID ");Serial.printf("%08X\n",(uint32_t)nodeID);
      send_ACK(0x02);
      }
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x03 && rdf.pack.did==(uint32_t)nodeID )  // received DATA frame 
      {
      Serial.print("Received DATA frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
      xQueueSend(dqueue, rdf.pack.sens, portMAX_DELAY);
      xQueueSend(tsqueue, &rdf.pack, portMAX_DELAY);
      send_ACK(0x04);
      }  
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
